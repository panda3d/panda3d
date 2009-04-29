from direct.stdpy.file import open
from pandac.PandaModules import Filename, VirtualFileSystem, VirtualFileMountSystem
import sys
import new
import os
import marshal
import imp
import struct
import __builtin__

__all__ = ['register']

vfs = VirtualFileSystem.getGlobalPtr()

# Possible file types.
FTPythonSource = 0
FTPythonCompiled = 1
FTCompiledModule = 2

pycExtension = 'pyc'
if not __debug__:
    # In optimized mode, we actually operate on .pyo files, not .pyc
    # files.
    pycExtension = 'pyo'

class VFSImporter:
    """ This class serves as a Python importer to support loading
    Python .py and .pyc/.pyo files from Panda's Virtual File System,
    which allows loading Python source files from mounted .mf files
    (among other places). """

    def __init__(self, path):
        self.dir_path = Filename.fromOsSpecific(path)

    def find_module(self, fullname):
        basename = fullname.split('.')[-1]
        path = Filename(self.dir_path, basename)

        # First, look for Python files.
        filename = Filename(path)
        filename.setExtension('py')
        vfile = vfs.getFile(filename, True)
        if vfile:
            return VFSLoader(self, vfile, filename, FTPythonSource)

        # If there's no .py file, but there's a .pyc file, load that
        # anyway.
        filename = Filename(path)
        filename.setExtension(pycExtension)
        vfile = vfs.getFile(filename, True)
        if vfile:
            return VFSLoader(self, vfile, filename, FTPythonCompiled)

        # Look for a compiled C/C++ module.
        for desc in imp.get_suffixes():
            if desc[2] != imp.C_EXTENSION:
                continue
            
            filename = Filename(path)
            filename.setExtension(desc[0][1:])
            vfile = vfs.getFile(filename, True)
            if vfile:
                return VFSLoader(self, vfile, filename, FTCompiledModule,
                                 desc = desc)
        

        # Finally, consider a package, i.e. a directory containing
        # __init__.py.
        filename = Filename(path, '__init__.py')
        vfile = vfs.getFile(filename, True)
        if vfile:
            return VFSLoader(self, vfile, filename, FTPythonSource,
                             packagePath = path)
        filename = Filename(path, '__init__.' + pycExtension)
        vfile = vfs.getFile(filename, True)
        if vfile:
            return VFSLoader(self, vfile, filename, FTPythonCompiled,
                             packagePath = path)

        return None

class VFSLoader:
    """ The second part of VFSImporter, this is created for a
    particular .py file or directory. """
    
    def __init__(self, importer, vfile, filename, fileType,
                 desc = None, packagePath = None):
        self.importer = importer
        self.dir_path = importer.dir_path
        self.timestamp = vfile.getTimestamp()
        self.filename = filename
        self.fileType = fileType
        self.desc = desc
        self.packagePath = packagePath
    
    def load_module(self, fullname):
        if self.fileType == FTCompiledModule:
            return self._import_compiled_module(fullname)
        
        code = self._read_code()
        if not code:
            raise ImportError
        
        mod = sys.modules.setdefault(fullname, new.module(fullname))
        mod.__file__ = self.filename.cStr()
        mod.__loader__ = self
        if self.packagePath:
            mod.__path__ = [self.packagePath.cStr()]

        exec code in mod.__dict__
        return mod

    def getdata(self, path):
        path = Filename(self.dir_path, Filename.fromOsSpecific(path))
        f = open(path, 'rb')
        return f.read()

    def is_package(self, fullname):
        return bool(self.packagePath)

    def get_code(self, fullname):
        return self._read_code()

    def get_source(self, fullname):
        return self._read_source()
        
    def _read_source(self):
        """ Returns the Python source for this file, if it is
        available, or None if it is not. """
        
        if self.fileType == FTPythonCompiled or \
           self.fileType == FTCompiledModule:
            return None
        
        filename = Filename(self.filename)
        filename.setExtension('py')
        try:
            file = open(filename, 'rU')
        except IOError:
            return None
        return file.read()

    def _import_compiled_module(self, fullname):
        """ Loads the compiled C/C++ shared object as a Python module,
        and returns it. """

        vfile = vfs.getFile(self.filename, False)

        # We can only import a compiled module if it already exists on
        # disk.  This means if it's a truly virtual file that has no
        # on-disk equivalent, we have to write it to a temporary file
        # first.
        if hasattr(vfile, 'getMount') and \
           isinstance(vfile.getMount(), VirtualFileMountSystem):
            # It's a real file.
            filename = self.filename
        else:
            # It's a virtual file.  Dump it.
            filename = Filename.temporary('', self.filename.getBasenameWoExtension(),
                                          '.' + self.filename.getExtension(),
                                          type = Filename.TDso)
            filename.setExtension(self.filename.getExtension())
            fin = open(vfile, 'rb')
            fout = open(filename, 'wb')
            data = fin.read(4096)
            while data:
                fout.write(data)
                data = fin.read(4096)
            fin.close()
            fout.close()

        module = imp.load_module(fullname, None, filename.toOsSpecific(),
                                 self.desc)
        module.__file__ = self.filename.cStr()
        return module
        

    def _read_code(self):
        """ Returns the Python compiled code object for this file, if
        it is available, or None if it is not. """

        if self.fileType == FTPythonCompiled:
            # It's a pyc file; just read it directly.
            pycVfile = vfs.getFile(self.filename, False)
            if pycVfile:
                return self._loadPyc(pycVfile, None)
            return None

        elif self.fileType == FTCompiledModule:
            return None

        # It's a .py file (or an __init__.py file; same thing).  Read
        # the .pyc file if it is available and current; otherwise read
        # the .py file and compile it.
        pycFilename = Filename(self.filename)
        pycFilename.setExtension(pycExtension)
        pycVfile = vfs.getFile(pycFilename, False)
        t_pyc = None
        if pycVfile:
            t_pyc = pycVfile.getTimestamp()

        code = None
        if t_pyc and t_pyc >= self.timestamp:
            code = self._loadPyc(pycVfile, self.timestamp)

        if not code:
            source = self._read_source()
            filename = Filename(self.filename)
            filename.setExtension('py')
            code = self._compile(filename, source)

        return code

    def _loadPyc(self, vfile, timestamp):
        """ Reads and returns the marshal data from a .pyc file. """
        code = None
        f = open(vfile, 'rb')
        if f.read(4) == imp.get_magic():
            t = struct.unpack('<I', f.read(4))[0]
            if not timestamp or t == timestamp:
                code = marshal.loads(f.read())
        f.close()
        return code
        

    def _compile(self, filename, source):
        """ Compiles the Python source code to a code object and
        attempts to write it to an appropriate .pyc file. """
        
        if source and source[-1] != '\n':
            source = source + '\n'
        code = __builtin__.compile(source, filename.cStr(), 'exec')

        # try to cache the compiled code
        pycFilename = Filename(filename)
        pycFilename.setExtension(pycExtension)
        try:
            f = open(pycFilename, 'wb')
        except IOError:
            pass
        else:
            f.write('\0\0\0\0')
            f.write(struct.pack('<I', self.timestamp))
            f.write(marshal.dumps(code))
            f.flush()
            f.seek(0, 0)
            f.write(imp.get_magic())
            f.close()

        return code

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
    
