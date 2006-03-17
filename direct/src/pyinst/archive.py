#
# Gordon McMillan (as inspired and influenced by Greg Stein)
#

# subclasses may not need marshal or struct, but since they're
# builtin, importing is safe.
#
# While an Archive is really an abstraction for any "filesystem
# within a file", it is tuned for use with imputil.FuncImporter.
# This assumes it contains python code objects, indexed by the
# the internal name (ie, no '.py').
# See carchive.py for a more general archive (contains anything)
# that can be understood by a C program.

import marshal
import struct

class Archive:
  """ A base class for a repository of python code objects.

      The get_code method is used by imputil.FuntionImporter
      to get code objects by name.
      Archives are flat namespaces, so conflict between module
      names in different packages are possible. Use a different
      Archive for each package.
  """
  MAGIC = 'PYL\0'
  HDRLEN = 12        # default is MAGIC followed by python's magic, int pos of toc
  TOCPOS = 8
  TRLLEN = 0        # default - no trailer
  TOCTMPLT = {}     #
  os = None
  def __init__(self, path=None, start=0):
    """
         Initialize an Archive. If path is omitted, it will be an empty Archive.
         start is the seek position within path where the Archive starts."""
    self.toc = None
    self.path = path
    self.start = start
    import imp
    self.pymagic = imp.get_magic()
    if path is not None:
      self.lib = open(self.path, 'rb')
      self.checkmagic()
      self.loadtoc()

  ####### Sub-methods of __init__ - override as needed #############
  def checkmagic(self):
    """Verify version and validity of file.

        Overridable.
        Check to see if the file object self.lib actually has a file
        we understand.
    """
    self.lib.seek(self.start)   #default - magic is at start of file
    if self.lib.read(len(self.MAGIC)) != self.MAGIC:
      raise RuntimeError, "%s is not a valid %s archive file" \
                % (self.path, self.__class__.__name__)
    if self.lib.read(len(self.pymagic)) != self.pymagic:
      raise RuntimeError, "%s has version mismatch to dll" % (self.path)

  def loadtoc(self):
    """Load the table of contents.

        Overridable.
        Default: After magic comes an int (4 byte native) giving the
        position of the TOC within self.lib.
        Default: The TOC is a marshal-able string.
    """
    self.lib.seek(self.start + self.TOCPOS)
    (offset,) = struct.unpack('=i', self.lib.read(4))
    self.lib.seek(self.start + offset)
    self.toc = marshal.load(self.lib)

  ######## This is what is called by FuncImporter #######
  ## Since an Archive is flat, we ignore parent and modname.

  def get_code(self, parent, modname, fqname):
    """The import hook.

       Called by imputil.FunctionImporter.
       Override extract to tune getting code from the Archive."""
    rslt = self.extract(fqname) # None if not found, (ispkg, code) otherwise
    if rslt is None:
      return None
    ispkg, code = rslt
    if ispkg:
      return ispkg, code, {'__path__': []}
    return rslt

  ####### Core method - Override as needed  #########
  def extract(self, name):
    """ Get the object corresponding to name, or None.

        NAME is the name as specified in an 'import name'.
        'import a.b' will become:
        extract('a') (return None because 'a' is not a code object)
        extract('a.__init__') (return a code object)
        extract('a.b') (return a code object)
        Default implementation:
          self.toc is a dict
          self.toc[name] is pos
          self.lib has the code object marshal-ed at pos
    """
    ispkg, pos = self.toc.get(name, (0, None))
    if pos is None:
      return None
    self.lib.seek(self.start + pos)
    return ispkg, marshal.load(self.lib)

  ########################################################################
  # Informational methods

  def contents(self):
    """Return a list of the contents.

       Default implementation assumes self.toc is a dict like object.
    """
    return self.toc.keys()

  ########################################################################
  # Building

  ####### Top level method - shouldn't need overriding #######
  def build(self, path, lTOC):
    """Create an archive file of name PATH from LTOC.

       lTOC is a 'logical TOC' - a list of (name, path, ...)
       where name is the internal (import) name,
       and path is a file to get the object from, eg './a.pyc'.
    """
    self.path = path
    self.lib = open(path, 'wb')
    #reserve space for the header
    if self.HDRLEN:
      self.lib.write('\0'*self.HDRLEN)

    #create an empty toc

    if type(self.TOCTMPLT) == type({}):
      self.toc = {}
    else:       # assume callable
      self.toc = self.TOCTMPLT()

    for tocentry in lTOC:
      self.add(tocentry)   # the guts of the archive

    tocpos = self.lib.tell()
    self.save_toc(tocpos)
    if self.TRLLEN:
      self.save_trailer(tocpos)
    if self.HDRLEN:
      self.update_headers(tocpos)
    self.lib.close()


  ####### manages keeping the internal TOC and the guts in sync #######
  def add(self, entry):
    """Add an entry to the archive.

      Override this to influence the mechanics of the Archive.
       Assumes entry is a seq beginning with (nm, pth, ...) where
       nm is the key by which we'll be asked for the object.
       pth is the name of where we find the object.
    """
    if self.os is None:
      import os
      self.os = os
    nm = entry[0]
    pth = entry[1]
    ispkg = self.os.path.splitext(self.os.path.basename(pth))[0] == '__init__'
    self.toc[nm] = (ispkg, self.lib.tell())
    f = open(entry[1], 'rb')
    f.seek(8)   #skip magic and timestamp
    self.lib.write(f.read())

  def save_toc(self, tocpos):
    """Save the table of contents.

       Default - toc is a dict
       Gets marshaled to self.lib
    """
    marshal.dump(self.toc, self.lib)

  def save_trailer(self, tocpos):
    """Placeholder for Archives with trailers."""
    pass

  def update_headers(self, tocpos):
    """Update any header data.

       Default header is  MAGIC + Python's magic + tocpos"""
    self.lib.seek(self.start)
    self.lib.write(self.MAGIC)
    self.lib.write(self.pymagic)
    self.lib.write(struct.pack('=i', tocpos))

##############################################################
#
# ZlibArchive - an archive with compressed entries
#

class ZlibArchive(Archive):
  """A subclass of Archive that compresses entries with zlib
     and uses a (marshalled) dict as a table of contents"""
  MAGIC = 'PYZ\0'
  TOCPOS = 8
  HDRLEN = 12
  TRLLEN = 0
  TOCTMPLT = {}
  LEVEL = 9

  def __init__(self, path=None, offset=0):
    Archive.__init__(self, path, offset)
    # dynamic import so not imported if not needed
    global zlib
    import zlib

  def extract(self, name):
    """Get the code object for NAME.

       Return None if name is not in the table of contents.
       Otherwise, return a tuple (ispkg, code)"""
    (ispkg, pos, lngth) = self.toc.get(name, (0, None, 0))
    if pos is None:
      return None
    self.lib.seek(self.start + pos)
    return ispkg, marshal.loads(zlib.decompress(self.lib.read(lngth)))

  def add(self, entry):
    """Add an entry.

       ENTRY is a sequence where entry[0] is name and entry[1] is full path name.
       zlib compress the code object, and build a toc entry"""
    if self.os is None:
      import os
      self.os = os
    nm = entry[0]
    pth = entry[1]
    ispkg = self.os.path.splitext(self.os.path.basename(pth))[0] == '__init__'
    f = open(pth, 'rb')
    f.seek(8)   #skip magic and timestamp
    obj = zlib.compress(f.read(), self.LEVEL)
    self.toc[nm] = (ispkg, self.lib.tell(), len(obj))
    self.lib.write(obj)

