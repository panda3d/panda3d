""" This module reimplements Python's file I/O mechanisms using Panda
constructs.  This enables Python to interface more easily with Panda's
virtual file system, and it also better-supports Panda's
SIMPLE_THREADS model, by avoiding blocking all threads while waiting
for I/O to complete. """

__all__ = [
    'file', 'open', 'listdir', 'walk', 'join',
    'isfile', 'isdir', 'exists', 'lexists', 'getmtime', 'getsize',
    'execfile',
    ]

from panda3d import core
import types

_vfs = core.VirtualFileSystem.getGlobalPtr()

class file:
    def __init__(self, filename, mode = 'r', bufsize = None,
                 autoUnwrap = False):
        self.__stream = None
        self.__needsVfsClose = False
        self.__reader = None
        self.__writer = None
        self.closed = True
        self.encoding = None
        self.errors = None
        self.__lastWrite = False

        self.mode = mode
        self.name = None
        self.filename = None
        self.newlines = None
        self.softspace = False

        readMode = False
        writeMode = False

        if isinstance(filename, core.Istream) or isinstance(filename, core.Ostream):
            # If we were given a stream instead of a filename, assign
            # it directly.
            self.__stream = filename
            readMode = isinstance(filename, core.Istream)
            writeMode = isinstance(filename, core.Ostream)

        elif isinstance(filename, core.VirtualFile):
            # We can also "open" a VirtualFile object for reading.
            self.__stream = filename.openReadFile(autoUnwrap)
            if not self.__stream:
                message = 'Could not read virtual file %s' % (repr(filename))
                raise IOError, message
            self.__needsVfsClose = True
            readMode = True

        else:
            # Otherwise, we must have been given a filename.  Open it.
            if isinstance(filename, types.StringTypes):
                # If a raw string is given, assume it's an os-specific
                # filename.
                filename = core.Filename.fromOsSpecific(filename)
            else:
                # If a Filename is given, make a writable copy anyway.
                filename = core.Filename(filename)

            self.filename = filename
            self.name = filename.toOsSpecific()

            binary = False
            if 'b' in mode:
                # Strip 'b'.  This means a binary file.
                i = mode.index('b')
                mode = mode[:i] + mode[i + 1:]
                binary = True

            if 'U' in mode:
                # Strip 'U'.  We don't use it; universal-newline support
                # is built into Panda, and can't be changed at runtime.
                i = mode.index('U')
                mode = mode[:i] + mode[i + 1:]
                binary = False

            if mode == '':
                mode = 'r'

            # Per Python docs, we insist this is true.
            assert mode[0] in 'rwa'

            if binary:
                filename.setBinary()
            else:
                filename.setText()

            # Actually open the streams.
            if mode == 'w':
                self.__stream = _vfs.openWriteFile(filename, autoUnwrap, True)
                if not self.__stream:
                    message = 'Could not open %s for writing' % (filename)
                    raise IOError, message
                writeMode = True

            elif mode == 'a':
                self.__stream = _vfs.openAppendFile(filename)
                if not self.__stream:
                    message = 'Could not open %s for writing' % (filename)
                    raise IOError, message
                writeMode = True

            elif mode == 'w+':
                self.__stream = _vfs.openReadWriteFile(filename, True)
                if not self.__stream:
                    message = 'Could not open %s for writing' % (filename)
                    raise IOError, message
                readMode = True
                writeMode = True

            elif mode == 'a+':
                self.__stream = _vfs.openReadAppendFile(filename)
                if not self.__stream:
                    message = 'Could not open %s for writing' % (filename)
                    raise IOError, message
                readMode = True
                writeMode = True

            elif mode == 'r+':
                self.__stream = _vfs.openReadWriteFile(filename, False)
                if not self.__stream:
                    message = 'Could not open %s for writing' % (filename)
                    raise IOError, message
                readMode = True
                writeMode = True

            elif mode == 'r':
                self.__stream = _vfs.openReadFile(filename, autoUnwrap)
                if not self.__stream:
                    if not _vfs.exists(filename):
                        message = 'No such file: %s' % (filename)
                    else:
                        message = 'Could not open %s for reading' % (filename)
                    raise IOError, message
                readMode = True

            self.__needsVfsClose = True

        if readMode:
            self.__reader = core.StreamReader(self.__stream, False)
        if writeMode:
            self.__writer = core.StreamWriter(self.__stream, False)
            self.__lastWrite = True

    def __del__(self):
        self.close()

    def close(self):
        if self.__needsVfsClose:
            if self.__reader and self.__writer:
                _vfs.closeReadWriteFile(self.__stream)
            elif self.__reader:
                _vfs.closeReadFile(self.__stream)
            else:  # self.__writer:
                _vfs.closeWriteFile(self.__stream)

            self.__needsVfsClose = False
        self.__stream = None
        self.__needsVfsClose = False
        self.__reader = None
        self.__writer = None

    def flush(self):
        if self.__stream:
            self.__stream.clear()  # clear eof flag
            self.__stream.flush()

    def __iter__(self):
        return self

    def next(self):
        line = self.readline()
        if line:
            return line
        raise StopIteration

    def read(self, size = -1):
        if not self.__reader:
            if not self.__writer:
                # The stream is not even open at all.
                message = 'I/O operation on closed file'
                raise ValueError, message
            # The stream is open only in write mode.
            message = 'Attempt to read from write-only stream'
            raise IOError, message

        self.__stream.clear()  # clear eof flag
        self.__lastWrite = False
        if size >= 0:
            result = self.__reader.extractBytes(size)
        else:
            # Read to end-of-file.
            result = ''
            while not self.__stream.eof():
                result += self.__reader.extractBytes(1024)
        return result

    def readline(self, size = -1):
        if not self.__reader:
            if not self.__writer:
                # The stream is not even open at all.
                message = 'I/O operation on closed file'
                raise ValueError, message
            # The stream is open only in write mode.
            message = 'Attempt to read from write-only stream'
            raise IOError, message

        self.__stream.clear()  # clear eof flag
        self.__lastWrite = False
        return self.__reader.readline()

    def readlines(self, sizehint = -1):
        lines = []
        line = self.readline()
        while line:
            lines.append(line)
            line = self.readline()
        return lines

    xreadlines = readlines

    def seek(self, offset, whence = 0):
        if self.__stream:
            self.__stream.clear()  # clear eof flag
        if self.__reader:
            self.__stream.seekg(offset, whence)
        if self.__writer:
            self.__stream.seekp(offset, whence)

    def tell(self):
        if self.__lastWrite:
            if self.__writer:
                return self.__stream.tellp()
        else:
            if self.__reader:
                return self.__stream.tellg()
        message = 'I/O operation on closed file'
        raise ValueError, message

    def truncate(self):
        """ Sorry, this isn't supported by Panda's low-level I/O,
        because it isn't supported by the standard C++ library. """
        raise NotImplementedError

    def write(self, str):
        if not self.__writer:
            if not self.__reader:
                # The stream is not even open at all.
                message = 'I/O operation on closed file'
                raise ValueError, message
            # The stream is open only in read mode.
            message = 'Attempt to write to read-only stream'
            raise IOError, message

        self.__stream.clear()  # clear eof flag
        self.__writer.appendData(str)
        self.__lastWrite = True

    def writelines(self, lines):
        if not self.__writer:
            if not self.__reader:
                # The stream is not even open at all.
                message = 'I/O operation on closed file'
                raise ValueError, message
            # The stream is open only in read mode.
            message = 'Attempt to write to read-only stream'
            raise IOError, message

        self.__stream.clear()  # clear eof flag
        for line in lines:
            self.__writer.appendData(line)
        self.__lastWrite = True

    def __enter__(self):
        return self

    def __exit__(self, t, v, tb):
        self.close()

open = file

def listdir(path):
    """ Implements os.listdir over vfs. """
    files = []
    dirlist = _vfs.scanDirectory(core.Filename.fromOsSpecific(path))
    if dirlist is None:
        message = 'No such file or directory: %s' % (path)
        raise OSError, message
    for file in dirlist:
        files.append(file.getFilename().getBasename())
    return files

def walk(top, topdown = True, onerror = None, followlinks = True):
    """ Implements os.walk over vfs.

    Note: we don't support onerror or followlinks; errors are ignored
    and links are always followed. """

    dirnames = []
    filenames = []

    dirlist = _vfs.scanDirectory(top)
    if dirlist:
        for file in dirlist:
            if file.isDirectory():
                dirnames.append(file.getFilename().getBasename())
            else:
                filenames.append(file.getFilename().getBasename())

    if topdown:
        yield (top, dirnames, filenames)

    for dir in dirnames:
        next = join(top, dir)
        for tuple in walk(next, topdown = topdown):
            yield tuple

    if not topdown:
        yield (top, dirnames, filenames)

def join(path, *args):
    for part in args:
        if part == '':
            continue

        if part.startswith('/'):
            path = part
        elif path.endswith('/'):
            path = path + part
        else:
            path = '/'.join((path, part))

    return path

def isfile(path):
    return _vfs.isRegularFile(core.Filename.fromOsSpecific(path))

def isdir(path):
    return _vfs.isDirectory(core.Filename.fromOsSpecific(path))

def exists(path):
    return _vfs.exists(core.Filename.fromOsSpecific(path))

def lexists(path):
    return _vfs.exists(core.Filename.fromOsSpecific(path))

def getmtime(path):
    file = _vfs.getFile(core.Filename.fromOsSpecific(path), True)
    if not file:
        raise os.error
    return file.getTimestamp()

def getsize(path):
    file = _vfs.getFile(core.Filename.fromOsSpecific(path), True)
    if not file:
        raise os.error
    return file.getFileSize()

def execfile(path, globals=None, locals=None):
    file = _vfs.getFile(core.Filename.fromOsSpecific(path), True)
    if not file:
        raise os.error

    data = file.readFile(False)
    exec(data, globals, locals)
