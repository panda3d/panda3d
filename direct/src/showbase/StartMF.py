""" This module can serve as a startup script to play a Panda
application packed within a .mf file.  Run via:

python StartMF.py app.mf

This is currently experimental, but the intent is to work towards a
prepackaged Panda distributable, designed to run applications exactly
like this.

"""

import sys
from direct.showbase import VFSImporter
from pandac.PandaModules import VirtualFileSystem, Filename, Multifile, ConfigPageManager, getModelPath
from direct.stdpy.file import file, open
import __builtin__

MultifileRoot = '/mf'

def runPackedApp(args):
    if not args:
        print "No Panda app specified.  Use:"
        print "python StartMF.py app.mf"
        sys.exit(1)

    vfs = VirtualFileSystem.getGlobalPtr()

    fname = Filename.fromOsSpecific(args[0])
    if not vfs.exists(fname):
        print "No such file: %s" % (args[0])
        sys.exit(1)

    mf = Multifile()
    if not mf.openRead(fname):
        print "Not a Panda Multifile: %s" % (args[0])
        sys.exit(1)

    # Clear *all* the mount points, including "/", so that we no
    # longer access the disk directly.
    vfs.unmountAll()

    # Mount the Multifile under /mf, by convention, and make that our
    # "current directory".
    vfs.mount(mf, MultifileRoot, vfs.MFReadOnly)
    vfs.chdir(MultifileRoot)

    # Make sure the directories on our standard Python path are mounted
    # read-only, so we can still load Python.
    for dirname in sys.path:
        vfs.mount(dirname, dirname, vfs.MFReadOnly)

    # Also mount some standard directories read-write (temporary and
    # app-data directories).
    tdir = Filename.temporary('', '')
    for dirname in set([ tdir.getDirname(),
                         Filename.getTempDirectory().cStr(),
                         Filename.getUserAppdataDirectory().cStr(),
                         Filename.getCommonAppdataDirectory().cStr() ]):
        vfs.mount(dirname, dirname, 0)

    # Now set up Python to import this stuff.
    VFSImporter.register()
    sys.path = [ MultifileRoot ] + sys.path

    # Put our root directory on the model-path and prc-path, too.
    getModelPath().prependDirectory(MultifileRoot)

    cpMgr = ConfigPageManager.getGlobalPtr()
    cpMgr.getSearchPath().prependDirectory(MultifileRoot)
    cpMgr.reloadImplicitPages()

    # Replace the builtin open and file symbols so code will get our
    # versions by default, which can open and read files out of the
    # multifile.
    __builtin__.file = file
    __builtin__.open = open

    import main

if __name__ == '__main__':
    runPackedApp(sys.argv[1:])
    
