from __future__ import print_function

import collections
import os
import pip
import sys
import subprocess
import zipfile
import struct
import io

import distutils.core
import distutils.dir_util
import distutils.file_util

from . import FreezeTool
from . import pefile
import panda3d.core as p3d


if 'basestring' not in globals():
    basestring = str


class build_apps(distutils.core.Command):
    user_options = [] # TODO

    def initialize_options(self):
        self.build_base = os.path.join(os.getcwd(), 'build')
        self.gui_apps = {}
        self.console_apps = {}
        self.copy_paths = []
        self.exclude_paths = []
        self.include_modules = {}
        self.exclude_modules = {}
        self.deploy_platforms = []
        self.requirements_path = './requirements.txt'
        self.pypi_extra_indexes = []
        self.build_scripts= {
            '.egg': ('.bam', 'egg2bam -o {1} {0}'),
        }
        self.exclude_dependencies = []

        # We keep track of the zip files we've opened.
        self._zip_files = {}

    def _get_zip_file(self, path):
        if path in self._zip_files:
            return self._zip_files[path]

        zip = zipfile.ZipFile(path)
        self._zip_files[path] = zip
        return zip

    def finalize_options(self):
        # TODO
        pass

    def run(self):
        if not self.deploy_platforms:
            platforms = [p3d.PandaSystem.get_platform()]
            use_wheels = False
        else:
            platforms = self.deploy_platforms
            use_wheels = True
        print("Building platforms: {0}".format(','.join(platforms)))

        for platform in platforms:
            self.build_runtimes(platform, use_wheels)

    def download_wheels(self, platform):
        """ Downloads panda3d wheels for the given platform using pip.
        These are special wheels that are expected to contain a deploy_libs
        directory containing the Python runtime libraries, which will be added
        to sys.path."""

        whldir = os.path.join(self.build_base, '__whl_cache__')
        abi_tag = pip.pep425tags.get_abi_tag()

        if 'u' in abi_tag and not platform.startswith('manylinux'):
            abi_tag = abi_tag.replace('u', '')

        pip_args = [
            'download',
            '-d', whldir,
            '-r', self.requirements_path,
            '--only-binary', ':all:',
            '--platform', platform,
            '--abi', abi_tag
        ]

        for index in self.pypi_extra_indexes:
            pip_args += ['--extra-index-url', index]

        pip.main(args=pip_args)

        wheelpaths = [os.path.join(whldir,i) for i in os.listdir(whldir) if platform in i]
        return wheelpaths

    def build_runtimes(self, platform, use_wheels):
        """ Builds the distributions for the given platform. """

        builddir = os.path.join(self.build_base, platform)

        if os.path.exists(builddir):
            distutils.dir_util.remove_tree(builddir)
        distutils.dir_util.mkpath(builddir)

        path = sys.path[:]
        p3dwhl = None

        if use_wheels:
            wheelpaths = self.download_wheels(platform)

            for whl in wheelpaths:
                if 'panda3d-' in whl:
                    p3dwhlfn = whl
                    p3dwhl = self._get_zip_file(p3dwhlfn)
                    break
            else:
                raise RuntimeError("Missing panda3d wheel")

            #whlfiles = {whl: self._get_zip_file(whl) for whl in wheelpaths}

            # Add whl files to the path so they are picked up by modulefinder
            for whl in wheelpaths:
                path.insert(0, whl)

            # Add deploy_libs from panda3d whl to the path
            path.insert(0, os.path.join(p3dwhlfn, 'deploy_libs'))

        # Create runtimes
        freezer_extras = set()
        freezer_modules = set()
        def create_runtime(appname, mainscript, use_console):
            freezer = FreezeTool.Freezer(platform=platform, path=path)
            freezer.addModule('__main__', filename=mainscript)
            for incmod in self.include_modules.get(appname, []) + self.include_modules.get('*', []):
                freezer.addModule(incmod)
            for exmod in self.exclude_modules.get(appname, []) + self.exclude_modules.get('*', []):
                freezer.excludeModule(exmod)
            freezer.done(addStartupModules=True)

            stub_name = 'deploy-stub'
            if platform.startswith('win'):
                if not use_console:
                    stub_name = 'deploy-stubw'
                stub_name += '.exe'

            if use_wheels:
                stub_file = p3dwhl.open('panda3d_tools/{0}'.format(stub_name))
            else:
                dtool_path = p3d.Filename(p3d.ExecutionEnvironment.get_dtool_name()).to_os_specific()
                stub_path = os.path.join(os.path.dirname(dtool_path), '..', 'bin', stub_name)
                stub_file = open(stub_path, 'rb')

            freezer.generateRuntimeFromStub(os.path.join(builddir, appname), stub_file)
            stub_file.close()

            freezer_extras.update(freezer.extras)
            freezer_modules.update(freezer.getAllModuleNames())

        for appname, scriptname in self.gui_apps.items():
            create_runtime(appname, scriptname, False)

        for appname, scriptname in self.console_apps.items():
            create_runtime(appname, scriptname, True)

        # Copy extension modules
        whl_modules = []
        whl_modules_ext = ''
        if use_wheels:
            # Get the module libs
            whl_modules = [
                i.replace('deploy_libs/', '') for i in p3dwhl.namelist() if i.startswith('deploy_libs/')
            ]

            # Pull off extension
            if whl_modules:
                whl_modules_ext = '.'.join(whl_modules[0].split('.')[1:])
            whl_modules = [i.split('.')[0] for i in whl_modules]

        # Make sure to copy any builtins that have shared objects in the deploy libs
        for mod in freezer_modules:
            if mod in whl_modules:
                freezer_extras.add((mod, None))

        #FIXME: this is a temporary hack to pick up libpandagl.
        for lib in p3dwhl.namelist():
            if lib.startswith('panda3d/libpandagl.'):
                source_path = os.path.join(p3dwhlfn, lib)
                target_path = os.path.join(builddir, os.path.basename(lib))
                search_path = [os.path.dirname(source_path)]
                self.copy_with_dependencies(source_path, target_path, search_path)

        # Copy any shared objects we need
        for module, source_path in freezer_extras:
            if source_path is not None:
                # Rename panda3d/core.pyd to panda3d.core.pyd
                basename = os.path.basename(source_path)
                if '.' in module:
                    basename = module.rsplit('.', 1)[0] + '.' + basename

                # Remove python version string
                if sys.version_info >= (3, 0):
                    parts = basename.split('.')
                    parts = parts[:-2] + parts[-1:]
                    basename = '.'.join(parts)
            else:
                # Builtin module, but might not be builtin in wheel libs, so double check
                if module in whl_modules:
                    source_path = '{0}/deploy_libs/{1}.{2}'.format(p3dwhlfn, module, whl_modules_ext)
                    basename = os.path.basename(source_path)
                else:
                    continue

            # If this is a dynamic library, search for dependencies.
            search_path = [os.path.dirname(source_path)]
            if use_wheels:
                search_path.append(os.path.join(p3dwhlfn, 'deploy_libs'))

            target_path = os.path.join(builddir, basename)
            self.copy_with_dependencies(source_path, target_path, search_path)

        # Copy Panda3D files
        etcdir = os.path.join(builddir, 'etc')
        if not use_wheels:
            # etc
            dtool_fn = p3d.Filename(p3d.ExecutionEnvironment.get_dtool_name())
            libdir = os.path.dirname(dtool_fn.to_os_specific())
            src = os.path.join(libdir, '..', 'etc')
            distutils.dir_util.copy_tree(src, etcdir)
        else:
            distutils.dir_util.mkpath(etcdir)

            # Combine prc files with libs and copy the whole list
            panda_files = [i for i in p3dwhl.namelist() if i.endswith('.prc')]
            for pf in panda_files:
                dstdir = etcdir if pf.endswith('.prc') else builddir
                target_path = os.path.join(dstdir, os.path.basename(pf))
                source_path = os.path.join(p3dwhlfn, pf)

                # If this is a dynamic library, search for dependencies.
                search_path = [os.path.dirname(source_path)]
                if use_wheels:
                    search_path.append(os.path.join(p3dwhlfn, 'deploy_libs'))

                self.copy_with_dependencies(source_path, target_path, search_path)

        # Copy Game Files
        ignore_copy_list = [
            '__pycache__',
            '*.pyc',
            '*.py',
        ]
        ignore_copy_list += self.exclude_paths
        ignore_copy_list = [p3d.GlobPattern(i) for i in ignore_copy_list]

        def check_pattern(src):
            for pattern in ignore_copy_list:
                # Normalize file paths across platforms
                path = p3d.Filename.from_os_specific(src).get_fullpath()
                #print("check ignore:", pattern, src, pattern.matches(path))
                if pattern.matches(path):
                    return True
            return False

        def dir_has_files(directory):
            files = [
                i for i in os.listdir(directory)
                if not check_pattern(os.path.join(directory, i))
            ]
            return bool(files)

        def copy_file(src, dst):
            src = os.path.normpath(src)
            dst = os.path.normpath(dst)

            if check_pattern(src):
                print("skipping file", src)
                return

            dst_dir = os.path.dirname(dst)
            if not os.path.exists(dst_dir):
                distutils.dir_util.mkpath(dst_dir)

            ext = os.path.splitext(src)[1]
            if not ext:
                ext = os.path.basename(src)
            dst_root = os.path.splitext(dst)[0]

            if ext in self.build_scripts:
                dst_ext, script = self.build_scripts[ext]
                dst = dst_root + dst_ext
                script = script.format(src, dst)
                print("using script:", script)
                subprocess.call(script.split())
            else:
                #print("Copy file", src, dst)
                distutils.file_util.copy_file(src, dst)

        def copy_dir(src, dst):
            for item in os.listdir(src):
                s = os.path.join(src, item)
                d = os.path.join(dst, item)
                if os.path.isfile(s):
                    copy_file(s, d)
                elif dir_has_files(s):
                    copy_dir(s, d)

        for path in self.copy_paths:
            if isinstance(path, basestring):
                src = dst = path
            else:
                src, dst = path
            dst = os.path.join(builddir, dst)

            if os.path.isfile(src):
                copy_file(src, dst)
            else:
                copy_dir(src, dst)

    def add_dependency(self, name, target_dir, search_path, referenced_by):
        """ Searches for the given DLL on the search path.  If it exists,
        copies it to the target_dir. """

        if os.path.exists(os.path.join(target_dir, name)):
            # We've already added it earlier.
            return

        if name in self.exclude_dependencies:
            return

        for dir in search_path:
            source_path = os.path.join(dir, name)

            if os.path.isfile(source_path):
                target_path = os.path.join(target_dir, name)
                self.copy_with_dependencies(source_path, target_path, search_path)
                return

            elif '.whl/' in source_path:
                # Check whether the file exists inside the wheel.
                whl, wf = source_path.split('.whl/')
                whl += '.whl'
                whlfile = self._get_zip_file(whl)

                # Look case-insensitively.
                namelist = whlfile.namelist()
                namelist_lower = [file.lower() for file in namelist]

                if wf.lower() in namelist_lower:
                    # We have a match.  Change it to the correct case.
                    wf = namelist[namelist_lower.index(wf.lower())]
                    source_path = '/'.join((whl, wf))
                    target_path = os.path.join(target_dir, os.path.basename(wf))
                    self.copy_with_dependencies(source_path, target_path, search_path)
                    return

        # If we didn't find it, look again, but case-insensitively.
        name_lower = name.lower()

        for dir in search_path:
            if os.path.isdir(dir):
                files = os.listdir(dir)
                files_lower = [file.lower() for file in files]

                if name_lower in files_lower:
                    name = files[files_lower.index(name_lower)]
                    source_path = os.path.join(dir, name)
                    target_path = os.path.join(target_dir, name)
                    self.copy_with_dependencies(source_path, target_path, search_path)

        # Warn if we can't find it, but only once.
        self.warn("could not find dependency {0} (referenced by {1})".format(name, referenced_by))
        self.exclude_dependencies.append(name)

    def copy_with_dependencies(self, source_path, target_path, search_path):
        """ Copies source_path to target_path.  It also scans source_path for
        any dependencies, which are located along the given search_path and
        copied to the same directory as target_path.

        source_path may be located inside a .whl file. """

        print("copying {0} -> {1}".format(os.path.relpath(source_path, self.build_base), os.path.relpath(target_path, self.build_base)))

        # Copy the file, and open it for analysis.
        if '.whl/' in source_path:
            # This was found in a wheel, extract it
            whl, wf = source_path.split('.whl/')
            whl += '.whl'
            whlfile = self._get_zip_file(whl)
            data = whlfile.read(wf)
            with open(target_path, 'wb') as f:
                f.write(data)
            # Wrap the data in a BytesIO, since we need to be able to seek in
            # the file; the stream returned by whlfile.open won't let us seek.
            fp = io.BytesIO(data)
        else:
            # Regular file, copy it
            distutils.file_util.copy_file(source_path, target_path)
            fp = open(target_path, 'rb')

        # What kind of magic does the file contain?
        deps = []
        magic = fp.read(4)
        if magic.startswith(b'MZ'):
            # It's a Windows DLL or EXE file.
            pe = pefile.PEFile()
            pe.read(fp)
            deps = pe.imports

        elif magic == b'\x7FELF':
            # Elf magic.  Used on (among others) Linux and FreeBSD.
            deps = self._read_dependencies_elf(fp)

        elif magic in (b'\xFE\xED\xFA\xCE', b'\xCE\xFA\xED\xFE',
                       b'\xFE\xED\xFA\xCF', b'\xCF\xFA\xED\xFE'):
            # A Mach-O file, as used on macOS.
            deps = self._read_dependencies_macho(fp)

        elif magic in (b'\xCA\xFE\xBA\xBE', b'\xBE\xBA\xFE\bCA'):
            # A fat file, containing multiple Mach-O binaries.  In the future,
            # we may want to extract the one containing the architecture we
            # are building for.
            deps = self._read_dependencies_fat(fp)

        # If we discovered any dependencies, recursively add those.
        if deps:
            target_dir = os.path.dirname(target_path)
            base = os.path.basename(target_path)
            for dep in deps:
                self.add_dependency(dep, target_dir, search_path, base)

    def _read_dependencies_elf(self, elf):
        """ Having read the first 4 bytes of the ELF file, fetches the
        dependent libraries and returns those as a list. """

        ident = elf.read(12)

        # Make sure we read in the correct endianness and integer size
        byte_order = "<>"[ord(ident[1:2]) - 1]
        elf_class = ord(ident[0:1]) - 1 # 0 = 32-bits, 1 = 64-bits
        header_struct = byte_order + ("HHIIIIIHHHHHH", "HHIQQQIHHHHHH")[elf_class]
        section_struct = byte_order + ("4xI8xIII8xI", "4xI16xQQI12xQ")[elf_class]
        dynamic_struct = byte_order + ("iI", "qQ")[elf_class]

        type, machine, version, entry, phoff, shoff, flags, ehsize, phentsize, phnum, shentsize, shnum, shstrndx \
          = struct.unpack(header_struct, elf.read(struct.calcsize(header_struct)))
        dynamic_sections = []
        string_tables = {}

        # Seek to the section header table and find the .dynamic section.
        elf.seek(shoff)
        for i in range(shnum):
            type, offset, size, link, entsize = struct.unpack_from(section_struct, elf.read(shentsize))
            if type == 6 and link != 0: # DYNAMIC type, links to string table
                dynamic_sections.append((offset, size, link, entsize))
                string_tables[link] = None

        # Read the relevant string tables.
        for idx in string_tables.keys():
            elf.seek(shoff + idx * shentsize)
            type, offset, size, link, entsize = struct.unpack_from(section_struct, elf.read(shentsize))
            if type != 3: continue
            elf.seek(offset)
            string_tables[idx] = elf.read(size)

        # Loop through the dynamic sections and rewrite it if it has an rpath/runpath.
        needed = []
        rpath = []
        for offset, size, link, entsize in dynamic_sections:
            elf.seek(offset)
            data = elf.read(entsize)
            tag, val = struct.unpack_from(dynamic_struct, data)

            # Read tags until we find a NULL tag.
            while tag != 0:
                if tag == 1: # A NEEDED entry.  Read it from the string table.
                    string = string_tables[link][val : string_tables[link].find(b'\0', val)]
                    needed.append(string.decode('utf-8'))

                elif tag == 15 or tag == 29:
                    # An RPATH or RUNPATH entry.
                    string = string_tables[link][val : string_tables[link].find(b'\0', val)]
                    rpath += string.split(b':')

                data = elf.read(entsize)
                tag, val = struct.unpack_from(dynamic_struct, data)
        elf.close()

        #TODO: should we respect the RPATH?  Clear it?  Warn about it?
        return needed

    def _read_dependencies_macho(self, fp):
        """ Having read the first 4 bytes of the Mach-O file, fetches the
        dependent libraries and returns those as a list. """

        cputype, cpusubtype, filetype, ncmds, sizeofcmds, flags = \
            struct.unpack('<IIIIII', fp.read(24))

        is_64 = (cputype & 0x1000000) != 0
        if is_64:
            fp.read(4)

        # After the header, we get a series of linker commands.  We just
        # iterate through them and gather up the LC_LOAD_DYLIB commands.
        load_dylibs = []
        for i in range(ncmds):
            cmd, cmdsize = struct.unpack('<II', fp.read(8))
            cmd_data = fp.read(cmdsize - 8)
            cmd &= ~0x80000000

            if cmd == 0x0c: # LC_LOAD_DYLIB
                dylib = cmd_data[16:].decode('ascii').split('\x00', 1)[0]
                if dylib.startswith('@loader_path/'):
                    dylib = dylib.replace('@loader_path/', '')
                load_dylibs.append(dylib)

        return load_dylibs

    def _read_dependencies_fat(self, fp):
        num_fat = struct.unpack('>I', fp.read(4))[0]
        if num_fat == 0:
            return []

        # After the header we get a table of executables in this fat file,
        # each one with a corresponding offset into the file.
        # We are just interested in the first one for now.
        cputype, cpusubtype, offset, size, align = \
            struct.unpack('>IIIII', fp.read(20))

        # Add 4, since it expects we've already read the magic.
        fp.seek(offset + 4)
        return self._read_dependencies_macho(fp)


class bdist_apps(distutils.core.Command):
    user_options = []

    def initialize_options(self):
        pass

    def finalize_options(self):
        pass

    def run(self):
        build_cmd = self.get_finalized_command('build_apps')
        if not build_cmd.deploy_platforms:
            platforms = [p3d.PandaSystem.get_platform()]
        else:
            platforms = build_cmd.deploy_platforms
        build_base = build_cmd.build_base

        self.run_command('build_apps')
        os.chdir(build_base)

        for platform in platforms:
            build_dir = os.path.join(build_base, platform)
            base_dir = self.distribution.get_name()
            temp_dir = os.path.join(build_base, base_dir)
            archive_format = 'gztar' if platform.startswith('linux') else 'zip'
            basename = '{}_{}'.format(self.distribution.get_fullname(), platform)

            if (os.path.exists(temp_dir)):
                distutils.dir_util.remove_tree(temp_dir)
            distutils.dir_util.copy_tree(build_dir, temp_dir)

            distutils.archive_util.make_archive(basename, archive_format, root_dir=build_base, base_dir=base_dir)

            distutils.dir_util.remove_tree(temp_dir)

def setup(**attrs):
    commandClasses = attrs.setdefault("cmdclass", {})
    commandClasses['build_apps'] = build_apps
    commandClasses['bdist_apps'] = bdist_apps
    distutils.core.setup(**attrs)
