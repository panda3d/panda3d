#!/usr/bin/env python
###############################################################################
# Name: _trash.py                                                             #
# Purpose: Multiplatform recycle/trash implementation                         #
# Author: Kevin D. Smith <Kevin.Smith@sixquickrun.com>                        #
# Copyright: (c) 2007 Cody Precord <staff@editra.org>                         #
# Licence: wxWindows Licence                                                  #
###############################################################################

"""
Platform independent Recycle Bin / Trash implementation

The moveToTrash function in this module takes a path or list of 
paths and moves them to the Recycle Bin / Trash directory depending
on the platform.  For UNIX platforms, the FreeDesktop specification
of Trash <http://www.ramendik.ru/docs/trashspec.html> is implemented.

Any errors while moving files results in some form of TrashException.

"""

__author__ = "Kevin D. Smith <Kevin.Smith@sixquickrun.com>"
__revision__ = "$Revision: 58361 $"
__scid__ = "$Id: Trash.py 58361 2009-01-24 19:43:27Z CJP $"

__all__ = ['MoveToTrash']

#-----------------------------------------------------------------------------#
# Imports
import os
import time
import platform
import shutil
import stat

OSX = WIN = False

# Determine platform, if it's not one of these assume UNIX/Linux
if platform.system().lower() in ['windows', 'microsoft']:
    WIN = True
    # Install recycle.exe binary
    import _winrecycle
    env = os.environ
    recycleexe = os.path.join(env.get('TEMP', env.get('TMP', env.get('windir','.'))),'recycle.exe')
    exe = open(recycleexe,'wb')
    exe.write(_winrecycle.recycle)
    exe.close()
    del exe
    del _winrecycle
elif platform.mac_ver()[0]:
    OSX = True

#-----------------------------------------------------------------------------#

class TrashError(Exception):
    pass
    
class TrashDirectoryError(TrashError):
    pass
    
class TrashMoveError(TrashError):
    pass
    
class TrashPermissionsError(TrashMoveError):
    pass

def MoveToTrash(paths):
    """
    Move the given paths to the trash can
    
    Required Arguments:
    paths -- path or list of paths to move to the trash can
    
    """
    # Make sure that we are always dealing with a list
    if isinstance(paths, basestring):
        paths = [paths] 
       
    # Get absolute paths and make sure files exist
    paths = [os.path.abspath(x) for x in paths 
                                if os.path.exists(os.path.abspath(x))]
    
    # Run the correct trash function
    if OSX:
        return _osxTrash(paths)
    elif WIN:
        return _winTrash(paths)
    else:
        return _unixTrash(paths)

def _ensurePermissions(path):
    """ Make sure we have permissions to read and delete path """
    if not os.access(path, os.R_OK|os.W_OK):
        try: os.chmod(path, stat.S_IWRITE|stat.S_IREAD)
        except (IOError, OSError): pass
    if not os.access(path, os.R_OK):
        raise TrashPermissionsError, ('You do not have permissions to read this path', path)
    if not os.access(path, os.W_OK):
        raise TrashPermissionsError, ('You do not have permissions to remove this path', path)

def _winTrash(paths):
    """ Move to windows recycle bin if possible """
    for path in paths:
        # See if we can even do this
        _ensurePermissions(path)
        try:
            rc = os.spawnv(os.P_WAIT, recycleexe, 
                          [os.path.basename(recycleexe)] + ['"%s"'%path])
            if rc:
                raise TrashMoveError, ('Could not move path', path, '%s' % rc)
        except (IOError, OSError), msg:
            raise TrashMoveError, ('Could not move path', path, msg)

def _osxTrash(paths):
    """ Move paths to OS X Trash can """
    trashdir = os.path.join(os.path.expanduser('~'),'.Trash')
    if not os.path.isdir(trashdir):
        raise TrashDirectoryError, ('Could not locate trash directory', trashdir)

    for path in paths:
        # See if we can even do this
        _ensurePermissions(path)

        # Generate new filename in trash
        origpath = newpath = os.path.join(trashdir, os.path.basename(path))
        while os.path.exists(newpath):
            newpath = origpath
            base, ext = os.path.splitext(newpath)
            newpath = '%s %s%s' % (base, time.strftime('%H-%M-%S'), ext)        

        # Move the path
        try: 
            shutil.move(path, newpath)
        except (OSError, IOError), msg:
            raise TrashMoveError, ('Could not move path', path, msg)

def _unixTrash(paths):
    """ 
    Move paths to FreeDesktop Trash can 
    
    See <http://www.ramendik.ru/docs/trashspec.html>
    
    """
    trashdir = os.path.join(os.environ.get('XDG_DATA_HOME', 
                            os.path.join(os.path.expanduser('~'),'.local','share')), 'Trash')

    # Create trash directories as needed
    try:
        os.makedirs(os.path.join(trashdir, 'files'))
    except (IOError, OSError):
        pass    

    try:
        os.makedirs(os.path.join(trashdir, 'info'))
    except (IOError, OSError):
        pass
    
    # Make sure that directories got created
    if not os.path.isdir(os.path.join(trashdir, 'files')):
        raise TrashDirectoryError, ('Could not locate trash directory', trashdir)
    if not os.path.isdir(os.path.join(trashdir, 'info')):
        raise TrashDirectoryError, ('Could not locate trash directory', trashdir)
    
    for path in paths:
        # See if we can even do this
        _ensurePermissions(path)
        
        # Create unique filename
        origpath = newpath = os.path.join(trashdir, 'files', 
                                          os.path.basename(path))
        while os.path.exists(newpath):
            newpath = origpath
            base, ext = os.path.splitext(newpath)
            newpath = '%s %s%s' % (base, time.strftime('%H-%M-%S'), ext)

        # Write info file
        try:
            root, base = os.path.split(newpath)
            infopath = os.path.join(os.path.dirname(root), 
                                    'info', base + '.trashinfo')
            info = open(infopath,'w')
            info.write('[Trash Info]\n')
            info.write('Path=%s\n' % path)
            info.write(time.strftime('DeletionDate=%Y%m%dT%H:%M:%S\n'))
            info.close()
        except (OSError, IOError), msg:
            try:
                os.remove(infopath)
            except:
                pass
            raise TrashMoveError, ('Could not move path', path, msg)

        # Move file
        try:
            shutil.move(path, newpath)
        except (OSError, IOError), msg:
            raise TrashMoveError, ('Could not move path', path, msg)

#-----------------------------------------------------------------------------#

if __name__ == '__main__':
    import sys
    args = sys.argv[1:]
    if args:
        moveToTrash(args)
