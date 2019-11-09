""" This module reimplements Python's file I/O mechanisms using Panda
constructs.  This enables Python to interface more easily with Panda's
virtual file system, and it also better-supports Panda's
SIMPLE_THREADS model, by avoiding blocking all threads while waiting
for I/O to complete. """

__all__ = [
    'open', 'listdir', 'walk', 'join',
    'isfile', 'isdir', 'exists', 'lexists', 'getmtime', 'getsize',
    'execfile',
    ]

from panda3d import core
import sys
import os
import io
import encodings
from posixpath import join

_vfs = core.VirtualFileSystem.getGlobalPtr()

if sys.version_info < (3, 0):
    # Python 3 defines these subtypes of IOError, but Python 2 doesn't.
    FileNotFoundError = IOError
    IsADirectoryError = IOError
    FileExistsError = IOError
    PermissionError = IOError

    unicodeType = unicode
    strType = str
else:
    unicodeType = str
    strType = ()


def open(file, mode='r', buffering=-1, encoding=None, errors=None, newline=None, closefd=True):
    """This function emulates the built-in Python open() function, additionally
    providing support for Panda's virtual file system.  It takes the same
    arguments as Python's built-in open() function.
    """

    if sys.version_info >= (3, 0):
        # Python 3 is much stricter than Python 2, which lets
        # unknown flags fall through.
        for ch in mode:
            if ch not in 'rwxabt+U':
                raise ValueError("invalid mode: '%s'" % (mode))

    creating = 'x' in mode
    writing = 'w' in mode
    appending = 'a' in mode
    updating = '+' in mode
    binary = 'b' in mode
    universal = 'U' in mode
    reading = universal or 'r' in mode

    if binary and 't' in mode:
        raise ValueError("can't have text and binary mode at once")

    if creating + reading + writing + appending > 1:
        raise ValueError("must have exactly one of create/read/write/append mode")

    if binary:
        if encoding:
            raise ValueError("binary mode doesn't take an encoding argument")
        if errors:
            raise ValueError("binary mode doesn't take an errors argument")
        if newline:
            raise ValueError("binary mode doesn't take a newline argument")

    if isinstance(file, core.Istream) or isinstance(file, core.Ostream):
        # If we were given a stream instead of a filename, assign
        # it directly.
        raw = StreamIOWrapper(file)
        raw.mode = mode

    else:
        vfile = None

        if isinstance(file, core.VirtualFile):
            # We can also "open" a VirtualFile object for reading.
            vfile = file
            filename = vfile.getFilename()
        elif isinstance(file, unicodeType):
            # If a raw string is given, assume it's an os-specific
            # filename.
            filename = core.Filename.fromOsSpecificW(file)
        elif isinstance(file, strType):
            filename = core.Filename.fromOsSpecific(file)
        else:
            # It's either a Filename object or an os.PathLike.
            # If a Filename is given, make a writable copy anyway.
            filename = core.Filename(file)

        if binary or sys.version_info >= (3, 0):
            filename.setBinary()
        else:
            filename.setText()

        if not vfile:
            vfile = _vfs.getFile(filename)

        if not vfile:
            if reading:
                raise FileNotFoundError("No such file or directory: '%s'" % (filename))

            vfile = _vfs.createFile(filename)
            if not vfile:
                raise IOError("Failed to create file: '%s'" % (filename))

        elif creating:
            # In 'creating' mode, we have to raise FileExistsError
            # if the file already exists.  Otherwise, it's the same
            # as 'writing' mode.
            raise FileExistsError("File exists: '%s'" % (filename))

        elif vfile.isDirectory():
            raise IsADirectoryError("Is a directory: '%s'" % (filename))

        # Actually open the streams.
        if reading:
            if updating:
                stream = vfile.openReadWriteFile(False)
            else:
                stream = vfile.openReadFile(False)

            if not stream:
                raise IOError("Could not open %s for reading" % (filename))

        elif writing or creating:
            if updating:
                stream = vfile.openReadWriteFile(True)
            else:
                stream = vfile.openWriteFile(False, True)

            if not stream:
                raise IOError("Could not open %s for writing" % (filename))

        elif appending:
            if updating:
                stream = vfile.openReadAppendFile()
            else:
                stream = vfile.openAppendFile()

            if not stream:
                raise IOError("Could not open %s for appending" % (filename))

        else:
            raise ValueError("Must have exactly one of create/read/write/append mode and at most one plus")

        raw = StreamIOWrapper(stream, needsVfsClose=True)
        raw.mode = mode
        raw.name = vfile.getFilename().toOsSpecific()

    # If a binary stream was requested, return the stream we've created.
    if binary:
        return raw

    # If we're in Python 2, we don't decode unicode strings by default.
    if not encoding and sys.version_info < (3, 0):
        return raw

    line_buffering = False
    if buffering == 1:
        line_buffering = True
    elif buffering == 0:
        raise ValueError("can't have unbuffered text I/O")

    # Otherwise, create a TextIOWrapper object to wrap it.
    wrapper = io.TextIOWrapper(raw, encoding, errors, newline, line_buffering)
    wrapper.mode = mode
    return wrapper


if sys.version_info < (3, 0):
    # Python 2 had an alias for open() called file().
    __all__.append('file')
    file = open


class StreamIOWrapper(io.IOBase):
    """ This is a file-like object that wraps around a C++ istream and/or
    ostream object.  It only deals with binary data; to work with text I/O,
    create an io.TextIOWrapper object around this, or use the open()
    function that is also provided with this module. """

    def __init__(self, stream, needsVfsClose=False):
        self.__stream = stream
        self.__needsVfsClose = needsVfsClose
        self.__reader = None
        self.__writer = None
        self.__lastWrite = False

        if isinstance(stream, core.Istream):
            self.__reader = core.StreamReader(stream, False)

        if isinstance(stream, core.Ostream):
            self.__writer = core.StreamWriter(stream, False)
            self.__lastWrite = True
            if sys.version_info >= (3, 0):
                # In Python 3, we use appendData, which only accepts bytes.
                self.__write = self.__writer.appendData
            else:
                # In Python 2.7, we also accept unicode objects, which are
                # implicitly converted to C++ strings.
                self.__write = self.__writer.write

    def __repr__(self):
        s = "<direct.stdpy.file.StreamIOWrapper"
        if hasattr(self, 'name'):
            s += " name='%s'" % (self.name)
        if hasattr(self, 'mode'):
            s += " mode='%s'" % (self.mode)
        s += ">"
        return s

    def readable(self):
        return self.__reader is not None

    def writable(self):
        return self.__writer is not None

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
        self.__reader = None
        self.__writer = None

    def flush(self):
        if self.__writer:
            self.__stream.clear()  # clear eof flag
            self.__stream.flush()

    def read(self, size=-1):
        if not self.__reader:
            if not self.__writer:
                # The stream is not even open at all.
                raise ValueError("I/O operation on closed file")

            # The stream is open only in write mode.
            raise IOError("Attempt to read from write-only stream")

        self.__stream.clear()  # clear eof flag
        self.__lastWrite = False
        if size is not None and size >= 0:
            return self.__reader.extractBytes(size)
        else:
            # Read to end-of-file.
            result = bytearray()
            while not self.__stream.eof():
                result += self.__reader.extractBytes(4096)
            return bytes(result)

    read1 = read

    def readline(self, size=-1):
        if not self.__reader:
            if not self.__writer:
                # The stream is not even open at all.
                raise ValueError("I/O operation on closed file")

            # The stream is open only in write mode.
            raise IOError("Attempt to read from write-only stream")

        self.__stream.clear()  # clear eof flag
        self.__lastWrite = False
        return self.__reader.readline()

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
        raise ValueError("I/O operation on closed file")

    def write(self, b):
        if not self.__writer:
            if not self.__reader:
                # The stream is not even open at all.
                raise ValueError("I/O operation on closed file")

            # The stream is open only in read mode.
            raise IOError("Attempt to write to read-only stream")

        self.__stream.clear()  # clear eof flag
        self.__write(b)
        self.__lastWrite = True

    def writelines(self, lines):
        if not self.__writer:
            if not self.__reader:
                # The stream is not even open at all.
                raise ValueError("I/O operation on closed file")

            # The stream is open only in read mode.
            raise IOError("Attempt to write to read-only stream")

        self.__stream.clear()  # clear eof flag
        for line in lines:
            self.__write(line)
        self.__lastWrite = True


def listdir(path):
    """ Implements os.listdir over vfs. """
    files = []
    dirlist = _vfs.scanDirectory(core.Filename.fromOsSpecific(path))
    if dirlist is None:
        raise OSError("No such file or directory: '%s'" % (path))

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
