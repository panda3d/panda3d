""" This module reimplements Python's native glob module using Panda
vfs constructs.  This enables Python to interface more easily with Panda's
virtual file system. """

import sys
import os
import fnmatch

from direct.stdpy import file

__all__ = ["glob", "iglob"]

def glob(pathname):
    """Return a list of paths matching a pathname pattern.

    The pattern may contain simple shell-style wildcards a la fnmatch.

    """
    return list(iglob(pathname))

def iglob(pathname):
    """Return an iterator which yields the paths matching a pathname pattern.

    The pattern may contain simple shell-style wildcards a la fnmatch.

    """
    if not has_magic(pathname):
        if file.lexists(pathname):
            yield pathname
        return
    dirname, basename = os.path.split(pathname)
    if not dirname:
        for name in glob1(os.curdir, basename):
            yield name
        return
    if has_magic(dirname):
        dirs = iglob(dirname)
    else:
        dirs = [dirname]
    if has_magic(basename):
        glob_in_dir = glob1
    else:
        glob_in_dir = glob0
    for dirname in dirs:
        for name in glob_in_dir(dirname, basename):
            yield os.path.join(dirname, name)

# These 2 helper functions non-recursively glob inside a literal directory.
# They return a list of basenames. repr(glob1) accepts a pattern while `glob0`
# takes a literal basename (so it only has to check for its existence).

def glob1(dirname, pattern):
    if not dirname:
        dirname = os.curdir
    if sys.version_info < (3, 0) and isinstance(pattern, unicode) and not isinstance(dirname, unicode):
        dirname = unicode(dirname, sys.getfilesystemencoding() or
                                   sys.getdefaultencoding())
    try:
        names = file.listdir(dirname)
    except os.error:
        return []
    if pattern[0] != '.':
        names = [x for x in names if x[0] != '.']
    return fnmatch.filter(names, pattern)

def glob0(dirname, basename):
    if basename == '':
        # repr(os.path.split()) returns an empty basename for paths ending with a
        # directory separator.  'q*x/' should match only directories.
        if file.isdir(dirname):
            return [basename]
    else:
        if file.lexists(os.path.join(dirname, basename)):
            return [basename]
    return []


def has_magic(s):
    if isinstance(s, bytes):
        return b'*' in s or b'?' in s or b'[' in s
    else:
        return '*' in s or '?' in s or '[' in s

def escape(pathname):
    drive, pathname = os.path.splitdrive(pathname)
    if sys.version_info >= (3, 0) and isinstance(pathname, bytes):
        newpath = bytearray(drive)
        for c in pathname:
            if c == 42 or c == 63 or c == 91:
                newpath += bytes((91, c, 93))
            else:
                newpath.append(c)
        return bytes(newpath)
    else:
        newpath = drive
        for c in pathname:
            if c == '*' or c == '?' or c == '[':
                newpath += '[' + c + ']'
            else:
                newpath += c
        return newpath
