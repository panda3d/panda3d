"""The VFS importer allows importing Python modules from Panda3D's virtual
file system, through Python's standard import mechanism.

Calling the :func:`register()` function to register the import hooks should be
sufficient to enable this functionality.
"""

__all__ = ['register', 'sharedPackages',
           'reloadSharedPackage', 'reloadSharedPackages']

from panda3d.core import Filename, VirtualFileSystem, VirtualFileMountSystem, OFileStream, copyStream
from direct.stdpy.file import open
import sys
import marshal
import imp
import types

#: The sharedPackages dictionary lists all of the "shared packages",
#: special Python packages that automatically span multiple directories
#: via magic in the VFSImporter.  You can make a package "shared"
#: simply by adding its name into this dictionary (and then calling
#: reloadSharedPackages() if it's already been imported).
#:
#: When a package name is in this dictionary at import time, *all*
#: instances of the package are located along sys.path, and merged into
#: a single Python module with a __path__ setting that represents the
#: union.  Thus, you can have a direct.showbase.foo in your own
#: application, and loading it won't shadow the system
#: direct.showbase.ShowBase which is in a different directory on disk.
sharedPackages = {}

vfs = VirtualFileSystem.getGlobalPtr()

compiledExtensions = [ 'pyc', 'pyo' ]
if not __debug__:
    # In optimized mode, we prefer loading .pyo files over .pyc files.
    # We implement that by reversing the extension names.
    compiledExtensions = [ 'pyo', 'pyc' ]


class VFSImporter:
    """ This class serves as a Python importer to support loading
    Python .py and .pyc/.pyo files from Panda's Virtual File System,
    which allows loading Python source files from mounted .mf files
    (among other places). """

    def __init__(self, path):
        if isinstance(path, Filename):
            self.dir_path = Filename(path)
        else:
            self.dir_path = Filename.fromOsSpecific(path)

    def find_module(self, fullname, path = None):
        if path is None:
            dir_path = self.dir_path
        else:
            dir_path = path
        #print >>sys.stderr, "find_module(%s), dir_path = %s" % (fullname, dir_path)
        basename = fullname.split('.')[-1]
        path = Filename(dir_path, basename)

        # First, look for Python files.
        filename = Filename(path)
        filename.setExtension('py')
        vfile = vfs.getFile(filename, True)
        if vfile:
            return VFSLoader(dir_path, vfile, filename,
                             desc=('.py', 'U' if sys.version_info < (3, 4) else 'r', imp.PY_SOURCE))

        # If there's no .py file, but there's a .pyc file, load that
        # anyway.
        for ext in compiledExtensions:
            filename = Filename(path)
            filename.setExtension(ext)
            vfile = vfs.getFile(filename, True)
            if vfile:
                return VFSLoader(dir_path, vfile, filename,
                                 desc=('.'+ext, 'rb', imp.PY_COMPILED))

        # Look for a C/C++ extension module.
        for desc in imp.get_suffixes():
            if desc[2] != imp.C_EXTENSION:
                continue

            filename = Filename(path + desc[0])
            vfile = vfs.getFile(filename, True)
            if vfile:
                return VFSLoader(dir_path, vfile, filename, desc=desc)

        # Finally, consider a package, i.e. a directory containing
        # __init__.py.
        filename = Filename(path, '__init__.py')
        vfile = vfs.getFile(filename, True)
        if vfile:
            return VFSLoader(dir_path, vfile, filename, packagePath=path,
                             desc=('.py', 'U' if sys.version_info < (3, 4) else 'r', imp.PY_SOURCE))
        for ext in compiledExtensions:
            filename = Filename(path, '__init__.' + ext)
            vfile = vfs.getFile(filename, True)
            if vfile:
                return VFSLoader(dir_path, vfile, filename, packagePath=path,
                                 desc=('.'+ext, 'rb', imp.PY_COMPILED))

        #print >>sys.stderr, "not found."
        return None

class VFSLoader:
    """ The second part of VFSImporter, this is created for a
    particular .py file or directory. """

    def __init__(self, dir_path, vfile, filename, desc, packagePath=None):
        self.dir_path = dir_path
        self.timestamp = None
        if vfile:
            self.timestamp = vfile.getTimestamp()
        self.filename = filename
        self.desc = desc
        self.packagePath = packagePath

    def load_module(self, fullname, loadingShared = False):
        #print >>sys.stderr, "load_module(%s), dir_path = %s, filename = %s" % (fullname, self.dir_path, self.filename)
        if self.desc[2] == imp.PY_FROZEN:
            return self._import_frozen_module(fullname)
        if self.desc[2] == imp.C_EXTENSION:
            return self._import_extension_module(fullname)

        # Check if this is a child of a shared package.
        if not loadingShared and self.packagePath and '.' in fullname:
            parentname = fullname.rsplit('.', 1)[0]
            if parentname in sharedPackages:
                # It is.  That means it's a shared package too.
                parent = sys.modules[parentname]
                path = getattr(parent, '__path__', None)
                importer = VFSSharedImporter()
                sharedPackages[fullname] = True
                loader = importer.find_module(fullname, path = path)
                assert loader
                return loader.load_module(fullname)

        code = self._read_code()
        if not code:
            raise ImportError('No Python code in %s' % (fullname))

        mod = sys.modules.setdefault(fullname, imp.new_module(fullname))
        mod.__file__ = self.filename.toOsSpecific()
        mod.__loader__ = self
        if self.packagePath:
            mod.__path__ = [self.packagePath.toOsSpecific()]
            #print >> sys.stderr, "loaded %s, path = %s" % (fullname, mod.__path__)

        exec(code, mod.__dict__)
        return sys.modules[fullname]

    def getdata(self, path):
        path = Filename(self.dir_path, Filename.fromOsSpecific(path))
        vfile = vfs.getFile(path)
        if not vfile:
            raise IOError("Could not find '%s'" % (path))
        return vfile.readFile(True)

    def is_package(self, fullname):
        return bool(self.packagePath)

    def get_code(self, fullname):
        return self._read_code()

    def get_source(self, fullname):
        return self._read_source()

    def get_filename(self, fullname):
        return self.filename.toOsSpecific()

    def _read_source(self):
        """ Returns the Python source for this file, if it is
        available, or None if it is not.  May raise IOError. """

        if self.desc[2] == imp.PY_COMPILED or \
           self.desc[2] == imp.C_EXTENSION:
            return None

        filename = Filename(self.filename)
        filename.setExtension('py')
        filename.setText()

        if sys.version_info >= (3, 0):
            # Use the tokenize module to detect the encoding.
            import tokenize
            fh = open(self.filename, 'rb')
            encoding, lines = tokenize.detect_encoding(fh.readline)
            return (b''.join(lines) + fh.read()).decode(encoding)
        else:
            return open(self.filename, self.desc[1]).read()

    def _import_extension_module(self, fullname):
        """ Loads the binary shared object as a Python module, and
        returns it. """

        vfile = vfs.getFile(self.filename, False)

        # We can only import an extension module if it already exists on
        # disk.  This means if it's a truly virtual file that has no
        # on-disk equivalent, we have to write it to a temporary file
        # first.
        if hasattr(vfile, 'getMount') and \
           isinstance(vfile.getMount(), VirtualFileMountSystem):
            # It's a real file.
            filename = self.filename
        elif self.filename.exists():
            # It's a virtual file, but it's shadowing a real file in
            # the same directory.  Assume they're the same, and load
            # the real one.
            filename = self.filename
        else:
            # It's a virtual file with no real-world existence.  Dump
            # it to disk.  TODO: clean up this filename.
            filename = Filename.temporary('', self.filename.getBasenameWoExtension(),
                                          '.' + self.filename.getExtension(),
                                          type = Filename.TDso)
            filename.setExtension(self.filename.getExtension())
            filename.setBinary()
            sin = vfile.openReadFile(True)
            sout = OFileStream()
            if not filename.openWrite(sout):
                raise IOError
            if not copyStream(sin, sout):
                raise IOError
            vfile.closeReadFile(sin)
            del sout

        module = imp.load_module(fullname, None, filename.toOsSpecific(),
                                 self.desc)
        module.__file__ = self.filename.toOsSpecific()
        return module

    def _import_frozen_module(self, fullname):
        """ Imports the frozen module without messing around with
        searching any more. """
        #print >>sys.stderr, "importing frozen %s" % (fullname)
        module = imp.load_module(fullname, None, fullname,
                                 ('', '', imp.PY_FROZEN))

        # Workaround for bug in Python 2.
        if getattr(module, '__path__', None) == fullname:
            module.__path__ = []
        return module

    def _read_code(self):
        """ Returns the Python compiled code object for this file, if
        it is available, or None if it is not.  May raise IOError,
        ValueError, SyntaxError, or a number of other errors generated
        by the low-level system. """

        if self.desc[2] == imp.PY_COMPILED:
            # It's a pyc file; just read it directly.
            pycVfile = vfs.getFile(self.filename, False)
            if pycVfile:
                return self._loadPyc(pycVfile, None)
            raise IOError('Could not read %s' % (self.filename))

        elif self.desc[2] == imp.C_EXTENSION:
            return None

        # It's a .py file (or an __init__.py file; same thing).  Read
        # the .pyc file if it is available and current; otherwise read
        # the .py file and compile it.
        t_pyc = None
        for ext in compiledExtensions:
            pycFilename = Filename(self.filename)
            pycFilename.setExtension(ext)
            pycVfile = vfs.getFile(pycFilename, False)
            if pycVfile:
                t_pyc = pycVfile.getTimestamp()
                break

        code = None
        if t_pyc and t_pyc >= self.timestamp:
            try:
                code = self._loadPyc(pycVfile, self.timestamp)
            except ValueError:
                code = None

        if not code:
            source = self._read_source()
            filename = Filename(self.filename)
            filename.setExtension('py')
            code = self._compile(filename, source)

        return code

    def _loadPyc(self, vfile, timestamp):
        """ Reads and returns the marshal data from a .pyc file.
        Raises ValueError if there is a problem. """

        code = None
        data = vfile.readFile(True)
        if data[:4] != imp.get_magic():
            raise ValueError("Bad magic number in %s" % (vfile))

        if sys.version_info >= (3, 0):
            t = int.from_bytes(data[4:8], 'little')
            data = data[12:]
        else:
            t = ord(data[4]) + (ord(data[5]) << 8) + \
               (ord(data[6]) << 16) + (ord(data[7]) << 24)
            data = data[8:]

        if not timestamp or t == timestamp:
            return marshal.loads(data)
        else:
            raise ValueError("Timestamp wrong on %s" % (vfile))


    def _compile(self, filename, source):
        """ Compiles the Python source code to a code object and
        attempts to write it to an appropriate .pyc file.  May raise
        SyntaxError or other errors generated by the compiler. """

        if source and source[-1] != '\n':
            source = source + '\n'
        code = compile(source, filename.toOsSpecific(), 'exec')

        # try to cache the compiled code
        pycFilename = Filename(filename)
        pycFilename.setExtension(compiledExtensions[0])
        try:
            f = open(pycFilename.toOsSpecific(), 'wb')
        except IOError:
            pass
        else:
            f.write(imp.get_magic())
            if sys.version_info >= (3, 0):
                f.write((self.timestamp & 0xffffffff).to_bytes(4, 'little'))
                f.write(b'\0\0\0\0')
            else:
                f.write(chr(self.timestamp & 0xff) +
                        chr((self.timestamp >> 8) & 0xff) +
                        chr((self.timestamp >> 16) & 0xff) +
                        chr((self.timestamp >> 24) & 0xff))
            f.write(marshal.dumps(code))
            f.close()

        return code


class VFSSharedImporter:
    """ This is a special importer that is added onto the meta_path
    list, so that it is called before sys.path is traversed.  It uses
    special logic to load one of the "shared" packages, by searching
    the entire sys.path for all instances of this shared package, and
    merging them. """

    def __init__(self):
        pass

    def find_module(self, fullname, path = None, reload = False):
        #print >>sys.stderr, "shared find_module(%s), path = %s" % (fullname, path)

        if fullname not in sharedPackages:
            # Not a shared package; fall back to normal import.
            return None

        if path is None:
            path = sys.path

        excludePaths = []
        if reload:
            # If reload is true, we are simply reloading the module,
            # looking for new paths to add.
            mod = sys.modules[fullname]
            excludePaths = getattr(mod, '_vfs_shared_path', None)
            if excludePaths is None:
                # If there isn't a _vfs_shared_path symbol already,
                # the module must have been loaded through
                # conventional means.  Try to guess which path it was
                # found on.
                d = self.getLoadedDirname(mod)
                excludePaths = [d]

        loaders = []
        for dir in path:
            if dir in excludePaths:
                continue

            importer = sys.path_importer_cache.get(dir, None)
            if importer is None:
                try:
                    importer = VFSImporter(dir)
                except ImportError:
                    continue

                sys.path_importer_cache[dir] = importer

            try:
                loader = importer.find_module(fullname)
                if not loader:
                    continue
            except ImportError:
                continue

            loaders.append(loader)

        if not loaders:
            return None
        return VFSSharedLoader(loaders, reload = reload)

    def getLoadedDirname(self, mod):
        """ Returns the directory name that the indicated
        conventionally-loaded module must have been loaded from. """

        if not getattr(mod, '__file__', None):
            return None

        fullname = mod.__name__
        dirname = Filename.fromOsSpecific(mod.__file__).getDirname()

        parentname = None
        basename = fullname
        if '.' in fullname:
            parentname, basename = fullname.rsplit('.', 1)

        path = None
        if parentname:
            parent = sys.modules[parentname]
            path = parent.__path__
        if path is None:
            path = sys.path

        for dir in path:
            pdir = str(Filename.fromOsSpecific(dir))
            if pdir + '/' + basename == dirname:
                # We found it!
                return dir

        # Couldn't figure it out.
        return None


class VFSSharedLoader:
    """ The second part of VFSSharedImporter, this imports a list of
    packages and combines them. """

    def __init__(self, loaders, reload):
        self.loaders = loaders
        self.reload = reload

    def load_module(self, fullname):
        #print >>sys.stderr, "shared load_module(%s), loaders = %s" % (fullname, map(lambda l: l.dir_path, self.loaders))

        mod = None
        message = None
        path = []
        vfs_shared_path = []
        if self.reload:
            mod = sys.modules[fullname]
            path = mod.__path__ or []
            if path == fullname:
                # Work around Python bug setting __path__ of frozen modules.
                path = []
            vfs_shared_path = getattr(mod, '_vfs_shared_path', [])

        for loader in self.loaders:
            try:
                mod = loader.load_module(fullname, loadingShared = True)
            except ImportError:
                etype, evalue, etraceback = sys.exc_info()
                print("%s on %s: %s" % (etype.__name__, fullname, evalue))
                if not message:
                    message = '%s: %s' % (fullname, evalue)
                continue
            for dir in getattr(mod, '__path__', []):
                if dir not in path:
                    path.append(dir)

        if mod is None:
            # If all of them failed to load, raise ImportError.
            raise ImportError(message)

        # If at least one of them loaded successfully, return the
        # union of loaded modules.
        mod.__path__ = path
        mod.__package__ = fullname

        # Also set this special symbol, which records that this is a
        # shared package, and also lists the paths we have already
        # loaded.
        mod._vfs_shared_path = vfs_shared_path + [l.dir_path for l in self.loaders]

        return mod


_registered = False
def register():
    """ Register the VFSImporter on the path_hooks, if it has not
    already been registered, so that future Python import statements
    will vector through here (and therefore will take advantage of
    Panda's virtual file system). """

    global _registered
    if not _registered:
        _registered = True
        sys.path_hooks.insert(0, VFSImporter)
        sys.meta_path.insert(0, VFSSharedImporter())

        # Blow away the importer cache, so we'll come back through the
        # VFSImporter for every folder in the future, even those
        # folders that previously were loaded directly.
        sys.path_importer_cache = {}


def reloadSharedPackage(mod):
    """ Reloads the specific module as a shared package, adding any
    new directories that might have appeared on the search path. """

    fullname = mod.__name__
    path = None
    if '.' in fullname:
        parentname = fullname.rsplit('.', 1)[0]
        parent = sys.modules[parentname]
        path = parent.__path__

    importer = VFSSharedImporter()
    loader = importer.find_module(fullname, path = path, reload = True)
    if loader:
        loader.load_module(fullname)

    # Also force any child packages to become shared packages, if
    # they aren't already.
    for basename, child in list(mod.__dict__.items()):
        if isinstance(child, types.ModuleType):
            childname = child.__name__
            if childname == fullname + '.' + basename and \
               hasattr(child, '__path__') and \
               childname not in sharedPackages:
                sharedPackages[childname] = True
                reloadSharedPackage(child)


def reloadSharedPackages():
    """ Walks through the sharedPackages list, and forces a reload of
    any modules on that list that have already been loaded.  This
    allows new directories to be added to the search path. """

    #print >> sys.stderr, "reloadSharedPackages, path = %s, sharedPackages = %s" % (sys.path, sharedPackages.keys())

    # Sort the list, just to make sure parent packages are reloaded
    # before child packages are.
    for fullname in sorted(sharedPackages.keys()):
        mod = sys.modules.get(fullname, None)
        if not mod:
            continue

        reloadSharedPackage(mod)
