""" Tools for manipulating Portable Executable files.

This can be used, for example, to extract a list of dependencies from an .exe
or .dll file, or to add version information and an icon resource to it. """

__all__ = ["PEFile"]

from struct import Struct, unpack, pack, pack_into
from collections import namedtuple
from array import array
import time
from io import BytesIO
import sys

if sys.version_info >= (3, 0):
    unicode = str
    unichr = chr

# Define some internally used structures.
RVASize = namedtuple('RVASize', ('addr', 'size'))
impdirtab = namedtuple('impdirtab', ('lookup', 'timdat', 'forward', 'name', 'impaddr'))
expdirtab = namedtuple('expdirtab', ('flags', 'timdat', 'majver', 'minver', 'name', 'ordinal_base', 'nentries', 'nnames', 'entries', 'names', 'ordinals'))


def _unpack_zstring(mem, offs=0):
    "Read a zero-terminated string from memory."
    c = mem[offs]
    str = ""
    while c:
        str += chr(c)
        offs += 1
        c = mem[offs]
    return str

def _unpack_wstring(mem, offs=0):
    "Read a UCS-2 string from memory."
    name_len, = unpack('<H', mem[offs:offs+2])
    name = ""
    for i in range(name_len):
        offs += 2
        name += unichr(*unpack('<H', mem[offs:offs+2]))
    return name

def _padded(n, boundary):
    align = n % boundary
    if align:
        n += boundary - align
    return n


class Section(object):
    _header = Struct('<8sIIIIIIHHI')

    modified = True

    def read_header(self, fp):
        name, vsize, vaddr, size, scnptr, relptr, lnnoptr, nreloc, nlnno, flags = \
            self._header.unpack(fp.read(40))

        self.name = name.rstrip(b'\x00')
        self.vaddr = vaddr # Base virtual address to map to.
        self.vsize = vsize
        self.offset = scnptr # Offset of the section in the file.
        self.size = size
        self.flags = flags

        self.modified = False

    def write_header(self, fp):
        fp.write(self._header.pack(self.name, self.vsize, self.vaddr,
                                   self.size, self.offset, 0, 0, 0, 0,
                                   self.flags))

    def __repr__(self):
        return "<section '%s' memory %x-%x>" % (self.name, self.vaddr, self.vaddr + self.vsize)

    def __gt__(self, other):
        return self.vaddr > other.vaddr

    def __lt__(self, other):
        return self.vaddr < other.vaddr


class DataResource(object):
    """ A resource entry in the resource table. """

    # Resource types.
    cursor = 1
    bitmap = 2
    icon = 3
    menu = 4
    dialog = 5
    string = 6
    font_directory = 7
    font = 8
    accelerator = 9
    rcdata = 10
    message_table = 11
    cursor_group = 12
    icon_group = 14
    version = 16
    dlg_include = 17
    plug_play = 19
    vxd = 20
    animated_cursor = 21
    animated_icon = 22
    html = 23
    manifest = 24

    def __init__(self):
        self._ident = ()
        self.data = None
        self.code_page = 0

    @property
    def encoding(self):
        if self.code_page == 0:
            return 'ascii'
        else:
            return 'cp%d' % (self.code_page)

    def get_data(self):
        return self.data

    def get_text(self, errors='strict'):
        return self.data.decode(self.encoding, errors)


class IconGroupResource(object):
    code_page = 0
    type = 14
    _entry = Struct('<BBBxHHIH')
    Icon = namedtuple('Icon', ('width', 'height', 'planes', 'bpp', 'size', 'id'))

    def __init__(self):
        self.icons = []

    def add_icon(self, *args, **kwargs):
        self.icons.append(self.Icon(*args, **kwargs))

    def get_data(self):
        data = bytearray(pack('<HHH', 0, 1, len(self.icons)))

        for width, height, planes, bpp, size, id in self.icons:
            colors = 1 << (planes * bpp)
            if colors >= 256:
                colors = 0
            if width >= 256:
                width = 0
            if height >= 256:
                height = 0
            data += self._entry.pack(width, height, colors, planes, bpp, size, id)
        return data

    def unpack_from(self, data, offs=0):
        type, count = unpack('<HH', data[offs+2:offs+6])
        offs += 6
        for i in range(count):
            width, height, colors, planes, bpp, size, id = \
                self._entry.unpack(data[offs:offs+14])
            if width == 0:
                width = 256
            if height == 0:
                height = 256
            self.icons.append(self.Icon(width, height, planes, bpp, size, id))
            offs += 14


class VersionInfoResource(object):
    code_page = 0
    type = 16

    def __init__(self):
        self.string_info = {}
        self.var_info = {}
        self.signature = 0xFEEF04BD
        self.struct_version = 0x10000
        self.file_version = (0, 0, 0, 0)
        self.product_version = (0, 0, 0, 0)
        self.file_flags_mask = 0x3f
        self.file_flags = 0
        self.file_os = 0x40004 # Windows NT
        self.file_type = 1 # Application
        self.file_subtype = 0
        self.file_date = (0, 0)

    def get_data(self):
        # The first part of the header is pretty much fixed - we'll go
        # back later to write the struct size.
        data = bytearray(b'\x00\x004\x00\x00\x00V\x00S\x00_\x00V\x00E\x00R\x00S\x00I\x00O\x00N\x00_\x00I\x00N\x00F\x00O\x00\x00\x00\x00\x00')
        data += pack('<13I', self.signature, self.struct_version,
                             self.file_version[1] | (self.file_version[0] << 16),
                             self.file_version[3] | (self.file_version[2] << 16),
                             self.product_version[1] | (self.product_version[0] << 16),
                             self.product_version[3] | (self.product_version[2] << 16),
                             self.file_flags_mask, self.file_flags,
                             self.file_os, self.file_type, self.file_subtype,
                             self.file_date[0], self.file_date[1])

        self._pack_info(data, 'StringFileInfo', self.string_info)
        self._pack_info(data, 'VarFileInfo', self.var_info)
        data[0:2] = pack('<H', len(data))
        return data

    def _pack_info(self, data, key, value):
        offset = len(data)

        if isinstance(value, dict):
            type = 1
            value_length = 0
        elif isinstance(value, bytes) or isinstance(value, unicode):
            type = 1
            value_length = len(value) * 2 + 2
        else:
            type = 0
            value_length = len(value)

        data += pack('<HHH', 0, value_length, type)

        for c in key:
            data += pack('<H', ord(c))
        data += b'\x00\x00'
        if len(data) & 2:
            data += b'\x00\x00'
        assert len(data) & 3 == 0

        if isinstance(value, dict):
            for key2, value2 in sorted(value.items(), key=lambda x:x[0]):
                self._pack_info(data, key2, value2)
        elif isinstance(value, bytes) or isinstance(value, unicode):
            for c in value:
                data += pack('<H', ord(c))
            data += b'\x00\x00'
        else:
            data += value
            if len(data) & 1:
                data += b'\x00'

        if len(data) & 2:
            data += b'\x00\x00'
        assert len(data) & 3 == 0

        data[offset:offset+2] = pack('<H', len(data) - offset)

    def unpack_from(self, data):
        length, value_length = unpack('<HH', data[0:4])
        offset = 40 + value_length + (value_length & 1)
        dwords = array('I')
        if sys.version_info >= (3, 2):
            dwords.frombytes(bytes(data[40:offset]))
        else:
            dwords.fromstring(bytes(data[40:offset]))

        if len(dwords) > 0:
            self.signature = dwords[0]
        if len(dwords) > 1:
            self.struct_version = dwords[1]
        if len(dwords) > 3:
            self.file_version = \
               (int(dwords[2] >> 16), int(dwords[2] & 0xffff),
                int(dwords[3] >> 16), int(dwords[3] & 0xffff))
        if len(dwords) > 5:
            self.product_version = \
               (int(dwords[4] >> 16), int(dwords[4] & 0xffff),
                int(dwords[5] >> 16), int(dwords[5] & 0xffff))
        if len(dwords) > 7:
            self.file_flags_mask = dwords[6]
            self.file_flags = dwords[7]
        if len(dwords) > 8:
            self.file_os = dwords[8]
        if len(dwords) > 9:
            self.file_type = dwords[9]
        if len(dwords) > 10:
            self.file_subtype = dwords[10]
        if len(dwords) > 12:
            self.file_date = (dwords[11], dwords[12])

        while offset < length:
            offset += self._unpack_info(self, data, offset)

    def __getitem__(self, key):
        if key == 'StringFileInfo':
            return self.string_info
        elif key == 'VarFileInfo':
            return self.var_info
        else:
            raise KeyError("%s does not exist" % (key))

    def __contains__(self, key):
        return key in ('StringFileInfo', 'VarFileInfo')

    def _unpack_info(self, dict, data, offset):
        length, value_length, type = unpack('<HHH', data[offset:offset+6])
        assert length > 0
        end = offset + length
        offset += 6
        key = ""
        c, = unpack('<H', data[offset:offset+2])
        offset += 2
        while c:
            key += unichr(c)
            c, = unpack('<H', data[offset:offset+2])
            offset += 2

        # Padding bytes to align value to 32-bit boundary.
        offset = _padded(offset, 4)

        if value_length > 0:
            # It contains a value.
            if type:
                # It's a wchar array value.
                value = u""
                c, = unpack('<H', data[offset:offset+2])
                offset += 2
                while c:
                    value += unichr(c)
                    c, = unpack('<H', data[offset:offset+2])
                    offset += 2
            else:
                # A binary value.
                value = bytes(data[offset:offset+value_length])
                offset += value_length
            dict[key] = value
        else:
            # It contains sub-entries.
            if key not in dict:
                dict[key] = {}
            subdict = dict[key]
            while offset < end:
                offset += self._unpack_info(subdict, data, offset)

        # Padding bytes to pad value to 32-bit boundary.
        return _padded(length, 4)


class ResourceTable(object):
    """ A table in the resource directory. """

    _header = Struct('<IIHHHH')

    def __init__(self, ident=()):
        self.flags = 0
        self.timdat = 0
        self.version = (0, 0)
        self._name_leaves = []
        self._id_leaves = []
        self._ident = ident
        self._strings_size = 0 # Amount of space occupied by table keys.
        self._descs_size = 0

    def __getitem__(self, key):
        if isinstance(key, int):
            leaves = self._id_leaves
        else:
            leaves = self._name_leaves

        i = 0
        while i < len(leaves):
            idname, leaf = leaves[i]
            if idname >= key:
                if key == idname:
                    return leaf
                break
            i += 1
        if not isinstance(key, int):
            self._strings_size += _padded(len(key) * 2 + 2, 4)
        leaf = ResourceTable(ident=self._ident + (key,))
        leaves.insert(i, (key, leaf))
        return leaf

    def __setitem__(self, key, value):
        """ Adds the given item to the table.  Maintains sort order. """
        if isinstance(key, int):
            leaves = self._id_leaves
        else:
            leaves = self._name_leaves

        if not isinstance(value, ResourceTable):
            self._descs_size += 16

        value._ident = self._ident + (key,)
        i = 0
        while i < len(leaves):
            idname, leaf = leaves[i]
            if idname >= key:
                if key == idname:
                    if not isinstance(leaves[i][1], ResourceTable):
                        self._descs_size -= 16
                    leaves[i] = (key, value)
                    return
                break
            i += 1
        if not isinstance(key, int):
            self._strings_size += _padded(len(key) * 2 + 2, 4)
        leaves.insert(i, (key, value))

    def __len__(self):
        return len(self._name_leaves) + len(self._id_leaves)

    def __iter__(self):
        keys = []
        for name, leaf in self._name_leaves:
            keys.append(name)
        for id, leaf in self._id_leaves:
            keys.append(id)
        return iter(keys)

    def items(self):
        return self._name_leaves + self._id_leaves

    def count_resources(self):
        """Counts all of the resources."""
        count = 0
        for key, leaf in self._name_leaves + self._id_leaves:
            if isinstance(leaf, ResourceTable):
                count += leaf.count_resources()
            else:
                count += 1
        return count

    def get_nested_tables(self):
        """Returns all tables in this table and subtables."""
        # First we yield child tables, then nested tables.  This is the
        # order in which pack_into assumes the tables will be written.
        for key, leaf in self._name_leaves + self._id_leaves:
            if isinstance(leaf, ResourceTable):
                yield leaf

        for key, leaf in self._name_leaves + self._id_leaves:
            if isinstance(leaf, ResourceTable):
                for table in leaf.get_nested_tables():
                    yield table

    def pack_header(self, data, offs):
        self._header.pack_into(data, offs, self.flags, self.timdat,
                               self.version[0], self.version[1],
                               len(self._name_leaves), len(self._id_leaves))

    def unpack_from(self, mem, addr=0, offs=0):
        start = addr + offs
        self.flags, self.timdat, majver, minver, nnames, nids = \
            self._header.unpack(mem[start:start+16])
        self.version = (majver, minver)
        start += 16

        # Subtables/entries specified by string name.
        self._name_leaves = []
        for i in range(nnames):
            name_p, data = unpack('<II', mem[start:start+8])
            if name_p & 0x80000000:
                name = _unpack_wstring(mem, addr + (name_p & 0x7fffffff))
            else:
                # Not sure what to do with this; I don't have a file with this.
                name = str(name_p)

            if data & 0x80000000:
                entry = ResourceTable(self._ident + (name,))
                entry.unpack_from(mem, addr, data & 0x7fffffff)
            else:
                entry = self._unpack_data_entry(mem, addr + data, ident=self._ident+(name,))
                self._descs_size += 16
            self._name_leaves.append((name, entry))
            self._strings_size += _padded(len(name) * 2 + 2, 4)
            start += 8

        # Subtables/entries specified by integer ID.
        self._id_leaves = []
        for i in range(nids):
            id, data = unpack('<II', mem[start:start+8])
            if data & 0x80000000:
                entry = ResourceTable(self._ident + (id,))
                entry.unpack_from(mem, addr, data & 0x7fffffff)
            else:
                entry = self._unpack_data_entry(mem, addr + data, ident=self._ident+(id,))
                self._descs_size += 16
            self._id_leaves.append((id, entry))
            start += 8

    def _unpack_data_entry(self, mem, addr, ident):
        rva, size, code_page = unpack('<III', mem[addr:addr+12])
        type, name, lang = ident
        #print("%s/%s/%s: %s [%s]" % (type, name, lang, size, code_page))

        data = mem[rva:rva+size]

        if type == VersionInfoResource.type:
            entry = VersionInfoResource()
            entry.unpack_from(data)
        elif type == IconGroupResource.type:
            entry = IconGroupResource()
            entry.unpack_from(data)
        else:
            entry = DataResource()
            entry.data = data
            entry.code_page = code_page


class PEFile(object):

    imports = ()

    def open(self, fn, mode='r'):
        if 'b' not in mode:
            mode += 'b'
        self.fp = open(fn, mode)
        self.read(self.fp)

    def close(self):
        self.fp.close()

    def read(self, fp):
        """ Reads a PE file from the given file object, which must be opened
        in binary mode. """

        # Read position of header.
        fp.seek(0x3c)
        offset, = unpack('<I', fp.read(4))

        fp.seek(offset)
        if fp.read(4) != b'PE\0\0':
            raise ValueError("Invalid PE file.")

        # Read the COFF header.
        self.machine, nscns, timdat, symptr, nsyms, opthdr, flags = \
            unpack('<HHIIIHH', fp.read(20))

        if nscns == 0:
            raise ValueError("No sections found.")

        if not opthdr:
            raise ValueError("No opthdr found.")

        # Read part of the opthdr.
        magic, self.code_size, self.initialized_size, self.uninitialized_size = \
            unpack('<HxxIII', fp.read(16))

        # Read alignments.
        fp.seek(16, 1)
        self.section_alignment, self.file_alignment = unpack('<II', fp.read(8))

        # Read header/image sizes.
        fp.seek(16, 1)
        self.image_size, self.header_size = unpack('<II', fp.read(8))

        if magic == 0x010b: # 32-bit.
            fp.seek(28, 1)
        elif magic == 0x20B: # 64-bit.
            fp.seek(44, 1)
        else:
            raise ValueError("unknown type 0x%x" % (magic))

        self.rva_offset = fp.tell()
        numrvas, = unpack('<I', fp.read(4))

        self.exp_rva = RVASize(0, 0)
        self.imp_rva = RVASize(0, 0)
        self.res_rva = RVASize(0, 0)

        # Locate the relevant tables in memory.
        if numrvas >= 1:
            self.exp_rva = RVASize(*unpack('<II', fp.read(8)))
        if numrvas >= 2:
            self.imp_rva = RVASize(*unpack('<II', fp.read(8)))
        if numrvas >= 3:
            self.res_rva = RVASize(*unpack('<II', fp.read(8)))

        # Skip the rest of the tables.
        if numrvas >= 4:
            fp.seek((numrvas - 3) * 8, 1)

        # Loop through the sections to find the ones containing our tables.
        self.sections = []
        for i in range(nscns):
            section = Section()
            section.read_header(fp)
            self.sections.append(section)

        self.sections.sort()

        # Read the sections into some kind of virtual memory.
        self.vmem = bytearray(self.sections[-1].vaddr + self.sections[-1].size)
        memview = memoryview(self.vmem)

        for section in self.sections:
            fp.seek(section.offset)
            fp.readinto(memview[section.vaddr:section.vaddr+section.size])

        # Read the import table.
        start = self.imp_rva.addr
        dir = impdirtab(*unpack('<IIIII', self.vmem[start:start+20]))

        imports = []
        while dir.name and dir.lookup:
            name = _unpack_zstring(self.vmem, dir.name)
            imports.append(name)

            start += 20
            dir = impdirtab(*unpack('<IIIII', self.vmem[start:start+20]))

        # Make it a tuple to indicate we don't support modifying it for now.
        self.imports = tuple(imports)

        # Read the resource tables from the .rsrc section.
        self.resources = ResourceTable()
        if self.res_rva.addr and self.res_rva.size:
            self.resources.unpack_from(self.vmem, self.res_rva.addr)

    def get_export_address(self, symbol_name):
        """ Finds the virtual address for a named export symbol. """

        if isinstance(symbol_name, bytes):
            symbol_name = symbol_name.decode('ascii')

        start = self.exp_rva.addr
        expdir = expdirtab(*unpack('<IIHHIIIIIII', self.vmem[start:start+40]))
        if expdir.nnames == 0 or expdir.ordinals == 0 or expdir.names == 0:
            return None

        nptr = expdir.names
        optr = expdir.ordinals
        for i in range(expdir.nnames):
            name_rva, = unpack('<I', self.vmem[nptr:nptr+4])
            ordinal, = unpack('<H', self.vmem[optr:optr+2])
            if name_rva != 0:
                name = _unpack_zstring(self.vmem, name_rva)
                if name == symbol_name:
                    assert ordinal >= 0 and ordinal < expdir.nentries
                    start = expdir.entries + 4 * ordinal
                    addr, = unpack('<I', self.vmem[start:start+4])
                    return addr
            nptr += 4
            optr += 2

    def get_address_offset(self, addr):
        """ Turns an address into a offset relative to the file beginning. """

        section = self.get_address_section(addr)
        if section is not None:
            return (addr - section.vaddr) + section.offset

    def get_address_section(self, addr):
        """ Returns the section that this virtual address belongs to. """

        for section in self.sections:
            if addr >= section.vaddr and addr < section.vaddr + section.size:
                return section

    def add_icon(self, icon, ordinal=2):
        """ Adds an icon resource from the given Icon object.  Requires
        calling add_resource_section() afterwards. """

        group = IconGroupResource()
        self.resources[group.type][ordinal][1033] = group

        images = sorted(icon.images.items(), key=lambda x:-x[0])
        id = 1

        # Write 8-bpp image headers for sizes under 256x256.
        for size, image in images:
            if size >= 256:
                continue

            xorsize = size
            if xorsize % 4 != 0:
                xorsize += 4 - (xorsize % 4)
            andsize = (size + 7) >> 3
            if andsize % 4 != 0:
                andsize += 4 - (andsize % 4)
            datasize = 40 + 256 * 4 + (xorsize + andsize) * size
            group.add_icon(size, size, 1, 8, datasize, id)

            buf = BytesIO()
            icon._write_bitmap(buf, image, size, 8)

            res = DataResource()
            res.data = buf.getvalue()
            self.resources[3][id][1033] = res
            id += 1

        # And now the 24/32 bpp versions.
        for size, image in images:
            if size > 256:
                continue

            # Calculate the size so we can write the offset within the file.
            if image.hasAlpha():
                bpp = 32
                xorsize = size * 4
            else:
                bpp = 24
                xorsize = size * 3 + (-(size * 3) & 3)
            andsize = (size + 7) >> 3
            if andsize % 4 != 0:
                andsize += 4 - (andsize % 4)
            datasize = 40 + (xorsize + andsize) * size

            buf = BytesIO()
            icon._write_bitmap(buf, image, size, bpp)

            res = DataResource()
            res.data = buf.getvalue()
            self.resources[3][id][1033] = res
            group.add_icon(size, size, 1, bpp, datasize, id)
            id += 1

    def add_section(self, name, flags, data):
        """ Adds a new section with the given name, flags and data.  The
        virtual address space is automatically resized to fit the new data.

        Returns the newly created Section object. """

        if isinstance(name, unicode):
            name = name.encode('ascii')

        section = Section()
        section.name = name
        section.flags = flags

        # Put it at the end of all the other sections.
        section.offset = 0
        for s in self.sections:
            section.offset = max(section.offset, s.offset + s.size)

        # Align the offset.
        section.offset = _padded(section.offset, self.file_alignment)

        # Find a place to put it in the virtual address space.
        section.vaddr = len(self.vmem)
        align = section.vaddr % self.section_alignment
        if align:
            pad = self.section_alignment - align
            self.vmem += bytearray(pad)
            section.vaddr += pad

        section.vsize = len(data)
        section.size = _padded(section.vsize, self.file_alignment)
        self.vmem += data
        self.sections.append(section)

        # Update the size tallies from the opthdr.
        self.image_size += _padded(section.vsize, self.section_alignment)
        if flags & 0x20:
            self.code_size += section.size
        if flags & 0x40:
            self.initialized_size += section.size
        if flags & 0x80:
            self.uninitialized_size += section.size

        return section

    def add_version_info(self, file_ver, product_ver, data, lang=1033, codepage=1200):
        """ Adds a version info resource to the file. """

        if "FileVersion" not in data:
            data["FileVersion"] = '.'.join(file_ver)
        if "ProductVersion" not in data:
            data["ProductVersion"] = '.'.join(product_ver)

        assert len(file_ver) == 4
        assert len(product_ver) == 4

        res = VersionInfoResource()
        res.file_version = file_ver
        res.product_version = product_ver
        res.string_info = {
            "%04x%04x" % (lang, codepage): data
        }
        res.var_info = {
            "Translation": bytearray(pack("<HH", lang, codepage))
        }

        self.resources[16][1][lang] = res

    def add_resource_section(self):
        """ Adds a resource section to the file containing the resources that
        were previously added via add_icon et al.  Assumes the file does not
        contain a resource section yet. """

        # Calculate how much space to reserve.
        tables = [self.resources] + list(self.resources.get_nested_tables())
        table_size = 0
        string_size = 0
        desc_size = 16 * self.resources.count_resources()

        for table in tables:
            table._offset = table_size
            table_size += 16 + 8 * len(table)
            string_size += table._strings_size
            desc_size += table._descs_size

        # Now write the actual data.
        tbl_offs = 0
        str_offs = table_size
        desc_offs = str_offs + string_size
        data_offs = desc_offs + desc_size
        data = bytearray(data_offs)
        data_addr = _padded(len(self.vmem), self.section_alignment) + data_offs

        for table in tables:
            table.pack_header(data, tbl_offs)

            tbl_offs += 16

            for name, leaf in table._name_leaves:
                if isinstance(leaf, ResourceTable):
                    pack_into('<II', data, tbl_offs, str_offs | 0x80000000, leaf._offset | 0x80000000)
                else:
                    pack_into('<II', data, tbl_offs, str_offs | 0x80000000, desc_offs)
                    resdata = leaf.get_data()
                    pack_into('<IIII', data, desc_offs, data_addr, len(resdata), leaf.code_page, 0)
                    data += resdata
                    desc_offs += 16
                    data_addr += len(resdata)
                    align = len(resdata) & 3
                    if align:
                        data += bytearray(4 - align)
                        data_addr += 4 - align
                tbl_offs += 8

                # Pack the name into the string table.
                pack_into('<H', data, str_offs, len(name))
                str_offs += 2
                for c in name:
                    pack_into('<H', data, str_offs, ord(c))
                    str_offs += 2
                str_offs = _padded(str_offs, 4)

            for id, leaf in table._id_leaves:
                if isinstance(leaf, ResourceTable):
                    pack_into('<II', data, tbl_offs, id, leaf._offset | 0x80000000)
                else:
                    pack_into('<II', data, tbl_offs, id, desc_offs)
                    resdata = leaf.get_data()
                    pack_into('<IIII', data, desc_offs, data_addr, len(resdata), leaf.code_page, 0)
                    data += resdata
                    desc_offs += 16
                    data_addr += len(resdata)
                    align = len(resdata) & 3
                    if align:
                        data += bytearray(4 - align)
                        data_addr += 4 - align
                tbl_offs += 8

        flags = 0x40000040 # readable, contains initialized data
        section = self.add_section('.rsrc', flags, data)
        self.res_rva = RVASize(section.vaddr, section.vsize)

    def write_changes(self):
        """ Assuming the file was opened in read-write mode, writes back the
        changes made via this class to the .exe file. """

        fp = self.fp
        # Read position of header.
        fp.seek(0x3c)
        offset, = unpack('<I', fp.read(4))

        fp.seek(offset)
        if fp.read(4) != b'PE\0\0':
            raise ValueError("Invalid PE file.")

        # Sync read/write pointer.  Necessary before write.  Bug in Python?
        fp.seek(fp.tell())

        # Rewrite the first part of the COFF header.
        timdat = int(time.time())
        fp.write(pack('<HHI', self.machine, len(self.sections), timdat))

        # Write calculated init and uninitialised sizes to the opthdr.
        fp.seek(16, 1)
        fp.write(pack('<III', self.code_size, self.initialized_size, self.uninitialized_size))

        # Same for the image and header size.
        fp.seek(40, 1)
        fp.write(pack('<II', self.image_size, self.header_size))

        # Write the modified RVA table.
        fp.seek(self.rva_offset)
        numrvas, = unpack('<I', fp.read(4))
        assert numrvas >= 3

        fp.seek(self.rva_offset + 4)
        if numrvas >= 1:
            fp.write(pack('<II', *self.exp_rva))
        if numrvas >= 2:
            fp.write(pack('<II', *self.imp_rva))
        if numrvas >= 3:
            fp.write(pack('<II', *self.res_rva))

        # Skip the rest of the tables.
        if numrvas >= 4:
            fp.seek((numrvas - 3) * 8, 1)

        # Write the modified section headers.
        for section in self.sections:
            section.write_header(fp)
            assert fp.tell() <= self.header_size

        # Write the section data of modified sections.
        for section in self.sections:
            if not section.modified:
                continue

            fp.seek(section.offset)
            size = min(section.vsize, section.size)
            fp.write(self.vmem[section.vaddr:section.vaddr+size])

            pad = section.size - size
            assert pad >= 0
            if pad > 0:
                fp.write(bytearray(pad))

            section.modified = False
