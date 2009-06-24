#! /usr/bin/env python

"""

This module is intended to be compiled into the Panda3D runtime
distributable, but it can also be run directly via the Python
interpreter.  It will run a Panda3D applet--a p3d file--that has
previously been generated via packp3d.py.

Usage:

  runp3d.py app.p3d [keyword=value [keyword=value ...] ]

The command-line keywords mimic the additional parameters that may
appear in HTML syntax when the p3d file appears on a web page.  These
are passed as given to the app, which may decide what to do with them.

See pack3d.py for a script that generates these p3d files.

"""

import sys
from direct.showbase import VFSImporter
from direct.showbase.DirectObject import DirectObject
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

class AppRunner(DirectObject):
    def __init__(self):
        DirectObject.__init__(self)
        
        self.packedAppEnvironmentInitialized = False
        self.gotWindow = False
        self.gotP3DFilename = False
        self.started = False
        self.windowPrc = None
        self.instanceId = None

        # This is the default requestFunc that is installed if we
        # never call setRequestFunc().
        def defaultRequestFunc(*args):
            print "Ignoring request func: %s" % (args,)
        self.requestFunc = defaultRequestFunc

    def initPackedAppEnvironment(self):
        """ This function sets up the Python environment suitably for
        running a packed app.  It should only run once in any given
        session (and it includes logic to ensure this). """

        if self.packedAppEnvironmentInitialized:
            return

        self.packedAppEnvironmentInitialized = True

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

    def startIfReady(self):
        if self.started:
            return

        if self.gotWindow and self.gotP3DFilename:
            self.started = True

            # Hang a hook so we know when the window is actually opened.
            self.acceptOnce('window-event', self.windowEvent)

            import main
            if hasattr(main, 'main') and callable(main.main):
                main.main()

    def setP3DFilename(self, p3dFilename, tokens = [],
                       instanceId = None):
        # One day we will have support for multiple instances within a
        # Python session.  Against that day, we save the instance ID
        # for this instance.
        self.instanceId = instanceId
        
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
        self.initPackedAppEnvironment()

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

        self.gotP3DFilename = True
        self.startIfReady()

    def setupWindow(self, windowType, x, y, width, height, parent):
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

        if self.windowPrc:
            unloadPrcFile(self.windowPrc)
        self.windowPrc = loadPrcFileData("setupWindow", data)

        self.gotWindow = True
        self.startIfReady()

    def setRequestFunc(self, func):
        """ This method is called by the plugin at startup to supply a
        function that can be used to deliver requests upstream, to the
        plugin, and thereby to the browser. """
        self.requestFunc = func

    def sendRequest(self, request, *args):
        self.requestFunc(self.instanceId, request, args)

    def windowEvent(self, win):
        print "Got window event in runp3d"

        self.sendRequest('notify', 'window_opened')

    def parseSysArgs(self):
        """ Converts sys.argv into (p3dFilename, tokens). """
        import getopt
        opts, args = getopt.getopt(sys.argv[1:], 'h')

        for option, value in opts:
            if option == '-h':
                print __doc__
                sys.exit(1)

        if not args or not args[0]:
            raise ArgumentError, "No Panda app specified.  Use:\nrunp3d.py app.p3d"

        tokens = []
        for token in args[1:]:
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
    runner = AppRunner()
    runner.gotWindow = True
    try:
        runner.setP3DFilename(*runner.parseSysArgs())
    except ArgumentError, e:
        print e.args[0]
        sys.exit(1)
    run()
