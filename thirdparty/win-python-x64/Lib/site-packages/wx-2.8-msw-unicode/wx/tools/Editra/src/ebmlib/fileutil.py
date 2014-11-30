###############################################################################
# Name: fileutil.py                                                           #
# Purpose: File Management Utilities.                                         #
# Author: Cody Precord <cprecord@editra.org>                                  #
# Copyright: (c) 2009 Cody Precord <staff@editra.org>                         #
# Licence: wxWindows Licence                                                  #
###############################################################################

"""
Editra Business Model Library: File Utilities

Utility functions for managing and working with files.

"""

__author__ = "Cody Precord <cprecord@editra.org>"
__svnid__ = "$Id: fileutil.py 67396 2011-04-05 20:01:01Z CJP $"
__revision__ = "$Revision: 67396 $"

__all__ = [ 'GetAbsPath', 'GetFileExtension', 'GetFileModTime', 'GetFileName',
            'GetFileSize', 'GetPathName', 'GetPathFromURI', 'GetUniqueName', 
            'IsLink', 'MakeNewFile', 'MakeNewFolder', 'PathExists',
            'ResolveRealPath', 'IsExecutable', 'Which', 'ComparePaths']

#-----------------------------------------------------------------------------#
# Imports
import os
import platform
import urllib2
import stat

UNIX = WIN = False
if platform.system().lower() in ['windows', 'microsoft']:
    WIN = True
    try:
        # Check for if win32 extensions are available
        import win32com.client as win32client
    except ImportError:
        win32client = None

    try:
        # Check for win32api
        import win32api
    except ImportError:
        win32api = None
else:
    UNIX = True

#-----------------------------------------------------------------------------#

def uri2path(func):
    """Decorator method to convert path arguments that may be uri's to
    real file system paths. Arg 0 must be a file path or uri.

    """
    def WrapURI(*args, **kwargs):
        args = list(args)
        args[0] = GetPathFromURI(args[0])
        return func(*args, **kwargs)

    WrapURI.__name__ = func.__name__
    WrapURI.__doc__ = func.__doc__
    return WrapURI

#-----------------------------------------------------------------------------#

def ComparePaths(path1, path2):
    """Determine whether the two given paths are equivalent
    @param path1: unicode
    @param path2: unicode
    @return: bool

    """
    path1 = GetAbsPath(path1)
    path2 = GetAbsPath(path2)
    if WIN:
        path1 = path1.lower()
        path2 = path2.lower()
    return path1 == path2

@uri2path
def GetAbsPath(path):
    """Get the absolute path of a file of a file.
    @param path: string
    @return: string
    @note: on windows if win32api is available short notation paths will be
           converted to the proper long name.
    
    """
    rpath = os.path.abspath(path)
    # Resolve short path notation on Windows when possible
    if WIN and win32api is not None and u"~" in rpath:
        try:
            rpath = win32api.GetLongPathNameW(rpath)
        except Exception:
            # Ignore errors from win32api calls
            pass
    return rpath

def GetFileExtension(file_str):
    """Gets last atom at end of string as extension if
    no extension whole string is returned
    @param file_str: path or file name to get extension from

    """
    return file_str.split('.')[-1]

def GetFileModTime(file_name):
    """Returns the time that the given file was last modified on
    @param file_name: path of file to get mtime of

    """
    try:
        mod_time = os.path.getmtime(file_name)
    except (OSError, EnvironmentError):
        mod_time = 0
    return mod_time

def GetFileName(path):
    """Gets last atom on end of string as filename
    @param path: full path to get filename from

    """
    return os.path.split(path)[-1]

@uri2path
def GetFileSize(path):
    """Get the size of the file at a given path
    @param path: Path to file
    @return: long

    """
    try:
        return os.stat(path)[stat.ST_SIZE]
    except:
        return 0

def GetPathFromURI(path):
    """Get a local path from a file:// uri
    @return: normalized path

    """
    if path.startswith(u"file:"):
        path = path.replace(u"file:", u"")
        path = path.lstrip(u"/")
        if platform.system().lower() in ('windows', 'microsoft'):
            path = path.replace(u"/", u"\\")
            if len(path) >= 2 and path[1] != u':':
                # A valid windows file uri should start with the drive
                # letter. If not make the assumption that it should be
                # the C: drive.
                path = u"C:\\\\" + path
        else:
            path = u"/" + path
        path = urllib2.unquote(path)

    return path

@uri2path
def GetPathName(path):
    """Gets the path minus filename
    @param path: full path to get base of

    """
    return os.path.split(path)[0]

@uri2path
def IsLink(path):
    """Is the file a link
    @return: bool

    """
    if WIN:
        return path.endswith(".lnk") or os.path.islink(path)
    else:
        return os.path.islink(path)

@uri2path
def PathExists(path):
    """Does the path exist.
    @param path: file path or uri
    @return: bool

    """
    return os.path.exists(path)

@uri2path
def IsExecutable(path):
    """Is the file at the given path an executable file
    @param path: file path
    @return: bool

    """
    return os.path.isfile(path) and os.access(path, os.X_OK)

@uri2path
def ResolveRealPath(link):
    """Return the real path of the link file
    @param link: path of link file
    @return: string

    """
    assert IsLink(link), "ResolveRealPath expects a link file!"
    realpath = link
    if WIN and win32client is not None:
        shell = win32client.Dispatch("WScript.Shell")
        shortcut = shell.CreateShortCut(link)
        realpath = shortcut.Targetpath
    else:
        realpath = os.path.realpath(link)
    return realpath

def Which(program):
    """Find the path of the given executable
    @param program: executable name (i.e 'python')
    @return: executable path or None

    """
    # Check local directory first
    if IsExecutable(program):
        return program
    else:
        # Start looking on the $PATH
        for path in os.environ["PATH"].split(os.pathsep):
            exe_file = os.path.join(path, program)
            if IsExecutable(exe_file):
                return exe_file        
    return None

#-----------------------------------------------------------------------------#

def GetUniqueName(path, name):
    """Make a file name that will be unique in case a file of the
    same name already exists at that path.
    @param path: Root path to folder of files destination
    @param name: desired file name base
    @return: string

    """
    tmpname = os.path.join(path, name)
    if os.path.exists(tmpname):
        if '.' not in name:
            ext = ''
            fbase = name
        else:
            ext = '.' + name.split('.')[-1]
            fbase = name[:-1 * len(ext)]

        inc = len([x for x in os.listdir(path) if x.startswith(fbase)])
        tmpname = os.path.join(path, "%s-%d%s" % (fbase, inc, ext))
        while os.path.exists(tmpname):
            inc = inc + 1
            tmpname = os.path.join(path, "%s-%d%s" % (fbase, inc, ext))

    return tmpname


#-----------------------------------------------------------------------------#

def MakeNewFile(path, name):
    """Make a new file at the given path with the given name.
    If the file already exists, the given name will be changed to
    a unique name in the form of name + -NUMBER + .extension
    @param path: path to directory to create file in
    @param name: desired name of file
    @return: Tuple of (success?, Path of new file OR Error message)

    """
    if not os.path.isdir(path):
        path = os.path.dirname(path)
    fname = GetUniqueName(path, name)

    try:
        open(fname, 'w').close()
    except (IOError, OSError), msg:
        return (False, str(msg))

    return (True, fname)

def MakeNewFolder(path, name):
    """Make a new folder at the given path with the given name.
    If the folder already exists, the given name will be changed to
    a unique name in the form of name + -NUMBER.
    @param path: path to create folder on
    @param name: desired name for folder
    @return: Tuple of (success?, new dirname OR Error message)

    """
    if not os.path.isdir(path):
        path = os.path.dirname(path)
    folder = GetUniqueName(path, name)
    try:
        os.mkdir(folder)
    except (OSError, IOError), msg:
        return (False, str(msg))

    return (True, folder)
