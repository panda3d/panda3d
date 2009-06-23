""" This module can serve as a startup script to play a Panda
application packed within a .mf file.  Run via:

python RunAppMF.py app.mf

This is currently experimental, but the intent is to work towards a
prepackaged Panda distributable, designed to run applications exactly
like this.

Also see MakeAppMF.py.

"""

import sys
from direct.showbase import VFSImporter
from pandac.PandaModules import VirtualFileSystem, Filename, Multifile, loadPrcFileData, getModelPath, HTTPClient
from direct.stdpy import file
from direct.task.TaskManagerGlobal import taskMgr
import os
import __builtin__

MultifileRoot = '/mf'

# This defines the default prc file that is implicitly loaded with an
# application.
AppPrcFilename = 'App.prc'
AppPrc = """
default-model-extension .bam
"""

class ArgumentError(AttributeError):
    pass

__packedAppEnvironmentInitialized = False

def initPackedAppEnvironment():
    """ This function sets up the Python environment suitably for
    running a packed app.  It should only run once in any given
    session (and it includes logic to ensure this). """

    global __packedAppEnvironmentInitialized
    if __packedAppEnvironmentInitialized:
        return

    __packedAppEnvironmentInitialized = True

    # We need to make sure sys.stdout maps to sys.stderr instead, so
    # if someone makes an unadorned print command within Python code,
    # it won't muck up the data stream between parent and child.
    sys.stdout = sys.stderr

    vfs = VirtualFileSystem.getGlobalPtr()

    # Clear *all* the mount points, including "/", so that we no
    # longer access the disk directly.
    vfs.unmountAll()

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

    # Load the implicit App.prc file.
    loadPrcFileData(AppPrcFilename, AppPrc)

    # Replace the builtin open and file symbols so user code will get
    # our versions by default, which can open and read files out of
    # the multifile.
    __builtin__.file = file.file
    __builtin__.open = file.open
    os.listdir = file.listdir
    os.walk = file.walk

    # Make "/mf" our "current directory", for running the multifiles
    # we plan to mount there.
    vfs.chdir(MultifileRoot)

gotWindow = False
gotP3DFilename = False
started = False
def startIfReady():
    global gotWindow, gotP3DFilename, started
    if started:
        return
    
    if gotWindow and gotP3DFilename:
        started = True
        import main
        if hasattr(main, 'main') and callable(main.main):
            main.main()

def setP3DFilename(p3dFilename, tokens = []):
    tokenDict = dict(tokens)
    fname = Filename.fromOsSpecific(p3dFilename)
    if not p3dFilename:
        # If we didn't get a literal filename, we have to download it
        # from the URL.  TODO: make this a smarter temporary filename?
        fname = Filename.temporary('', 'p3d_')
        fname.setExtension('p3d')
        p3dFilename = fname.toOsSpecific()
        src = tokenDict.get('src', None)
        if not src:
            raise ArgumentError, "No Panda app specified."
            
        http = HTTPClient.getGlobalPtr()
        hc = http.getDocument(src)
        if not hc.downloadToFile(fname):
            fname.unlink()
            raise ArgumentError, "Couldn't download %s" % (src)

        # Set a hook on sys.exit to delete the temporary file.
        oldexitfunc = getattr(sys, 'exitfunc', None)
        def deleteTempFile(fname = fname, oldexitfunc = oldexitfunc):
            fname.unlink()
            if oldexitfunc:
                oldexitfunc()

        sys.exitfunc = deleteTempFile

    vfs = VirtualFileSystem.getGlobalPtr()

    if not vfs.exists(fname):
        raise ArgumentError, "No such file: %s" % (p3dFilename)

    fname.makeAbsolute()
    initPackedAppEnvironment()

    mf = Multifile()
    if not mf.openRead(fname):
        raise ArgumentError, "Not a Panda Multifile: %s" % (p3dFilename)

    # Mount the Multifile under /mf, by convention.
    vfs.mount(mf, MultifileRoot, vfs.MFReadOnly)

    # Load any prc files in the root.  We have to load them
    # explicitly, since the ConfigPageManager can't directly look
    # inside the vfs.  Use the Multifile interface to find the prc
    # files, rather than vfs.scanDirectory(), so we only pick up the
    # files in this particular multifile.
    for f in mf.getSubfileNames():
        fn = Filename(f)
        if fn.getDirname() == '' and fn.getExtension() == 'prc':
            pathname = '%s/%s' % (MultifileRoot, f)
            data = open(pathname, 'r').read()
            loadPrcFileData(pathname, data)

    global gotP3DFilename
    gotP3DFilename = True
    startIfReady()

windowPrc = None
def setupWindow(windowType, x, y, width, height, parent):
    if windowType == 'hidden':
        data = 'window-type none\n'
    else:
        data = 'window-type onscreen\n'

    if windowType == 'fullscreen':
        data += 'fullscreen 1\n'
    else:
        data += 'fullscreen 0\n'

    if windowType == 'embedded':
        data += 'parent-window-handle %s\n' % (parent)
    else:
        data += 'parent-window-handle 0\n'
    
    if x or y:
        data += 'win-origin %s %s\n' % (x, y)
    if width or height:
        data += 'win-size %s %s\n' % (width, height)

    global windowPrc
    if windowPrc:
        unloadPrcFile(windowPrc)
    windowPrc = loadPrcFileData("setupWindow", data)

    global gotWindow
    gotWindow = True
    startIfReady()

def parseSysArgs():
    """ Converts sys.argv into (p3dFilename, tokens). """
    if len(sys.argv) < 2 or not sys.argv[1]:
        raise ArgumentError, "No Panda app specified.  Use:\npython RunAppMF.py app.mf"

    tokens = []
    for token in sys.argv[2:]:
        if '=' in token:
            keyword, value = token.split('=', 1)
        else:
            keyword = token
            value = ''
        tokens.append((keyword.lower(), value))

    p3dFilename = Filename.fromOsSpecific(sys.argv[1])
    osFilename = p3dFilename.toOsSpecific()
    if not p3dFilename.exists():
        # If the filename doesn't exist, it must be a URL.
        osFilename = ''
        if 'src' not in dict(tokens):
            tokens.append(('src', sys.argv[1]))

    return (osFilename, tokens)
        
        

if __name__ == '__main__':
    gotWindow = True
    try:
        setP3DFilename(*parseSysArgs())
    except ArgumentError, e:
        print e.args[0]
        sys.exit(1)
    run()
