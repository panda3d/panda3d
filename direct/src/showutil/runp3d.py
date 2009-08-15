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
from pandac.PandaModules import VirtualFileSystem, Filename, Multifile, loadPrcFileData, unloadPrcFile, getModelPath, HTTPClient, Thread, WindowProperties, readXmlStream, ExecutionEnvironment
from direct.stdpy import file
from direct.task.TaskManagerGlobal import taskMgr
from direct.showbase.MessengerGlobal import messenger
from direct.showbase import AppRunnerGlobal

# These imports are read by the C++ wrapper in p3dPythonRun.cxx.
from direct.showutil.JavaScript import UndefinedObject, Undefined, ConcreteStruct, BrowserObject

import os
import types
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

class ScriptAttributes:
    """ This dummy class serves as the root object for the scripting
    interface.  The Python code can store objects and functions here
    for direct inspection by the browser's JavaScript code. """
    pass

class AppRunner(DirectObject):
    def __init__(self):
        DirectObject.__init__(self)

        self.sessionId = 0
        self.packedAppEnvironmentInitialized = False
        self.gotWindow = False
        self.gotP3DFilename = False
        self.started = False
        self.windowOpened = False
        self.windowPrc = None

        self.fullDiskAccess = False

        self.Undefined = Undefined
        self.ConcreteStruct = ConcreteStruct

        # This is per session.
        self.nextScriptId = 0

        # TODO: we need one of these per instance, not per session.
        self.instanceId = None

        # The attributes of this object will be exposed as attributes
        # of the plugin instance in the DOM.
        self.attributes = ScriptAttributes()

        # By default, we publish a stop() method so the browser can
        # easy stop the plugin.
        self.attributes.stop = self.stop

        # This will be the browser's toplevel window DOM object;
        # e.g. self.dom.document will be the document.
        self.dom = None

        # This is the list of expressions we will evaluate when
        # self.dom gets assigned.
        self.deferredEvals = []

        # This is the default requestFunc that is installed if we
        # never call setRequestFunc().
        def defaultRequestFunc(*args):
            if args[1] == 'notify':
                # Quietly ignore notifies.
                return
            print "Ignoring request: %s" % (args,)
        self.requestFunc = defaultRequestFunc

        # Store our pointer so DirectStart-based apps can find us.
        if AppRunnerGlobal.appRunner is None:
            AppRunnerGlobal.appRunner = self

        # We use this messenger hook to dispatch this startIfReady()
        # call back to the main thread.
        self.accept('startIfReady', self.startIfReady)
            
    def stop(self):
        """ This method can be called by JavaScript to stop the
        application. """

        # We defer the actual exit for a few frames, so we don't raise
        # an exception and invalidate the JavaScript call; and also to
        # help protect against race conditions as the application
        # shuts down.
        taskMgr.doMethodLater(0.5, sys.exit, 'exit')

    def setSessionId(self, sessionId):
        """ This message should come in at startup. """
        self.sessionId = sessionId
        self.nextScriptId = self.sessionId * 1000 + 10000

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

        # Unmount directories we don't need.  This doesn't provide
        # actual security, since it only disables this stuff for users
        # who go through the vfs; a malicious programmer can always
        # get to the underlying true file I/O operations.  Still, it
        # can help prevent honest developers from accidentally getting
        # stuck where they don't belong.
        if not self.fullDiskAccess:
            # Clear *all* the mount points, including "/", so that we
            # no longer access the disk directly.
            vfs.unmountAll()

            # Make sure the directories on our standard Python path
            # are mounted read-only, so we can still load Python.
            # Note: read-only actually doesn't have any effect on the
            # vfs right now; careless application code can still write
            # to these directories inadvertently.
            for dirname in sys.path:
                vfs.mount(dirname, dirname, vfs.MFReadOnly)

            # Also mount some standard directories read-write
            # (temporary and app-data directories).
            tdir = Filename.temporary('', '')
            for dirname in set([ tdir.getDirname(),
                                 Filename.getTempDirectory().cStr(),
                                 Filename.getUserAppdataDirectory().cStr(),
                                 Filename.getCommonAppdataDirectory().cStr() ]):
                vfs.mount(dirname, dirname, 0)

            # And we might need the current working directory.
            dirname = ExecutionEnvironment.getCwd()
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

            # Now we can ignore future calls to startIfReady().
            self.ignore('startIfReady')

            # Hang a hook so we know when the window is actually opened.
            self.acceptOnce('window-event', self.windowEvent)

            # Look for the startup Python file.  This may be a magic
            # filename (like "__main__", or any filename that contains
            # invalid module characters), so we can't just import it
            # directly; instead, we go through the low-level importer.

            # If there's no p3d_info.xml file, we look for "main".
            moduleName = 'main'
            if self.p3dPackage:
                mainName = self.p3dPackage.Attribute('main_module')
                if mainName:
                    moduleName = mainName
            
            v = VFSImporter.VFSImporter(MultifileRoot)
            loader = v.find_module(moduleName)
            if not loader:
                message = "No %s.py found in application." % (mainName)
                raise StandardError, message
            
            main = loader.load_module(moduleName)
            if hasattr(main, 'main') and callable(main.main):
                main.main()

    def getPandaScriptObject(self):
        """ Called by the browser to query the Panda instance's
        toplevel scripting object, for querying properties in the
        Panda instance.  The attributes on this object are mapped to
        the plugin instance within the DOM. """
        return self.attributes

    def setBrowserScriptObject(self, dom):
        """ Called by the browser to supply the browser's toplevel DOM
        object, for controlling the JavaScript and the document in the
        same page with the Panda3D plugin. """

        self.dom = dom
        print "setBrowserScriptObject(%s)" % (dom)

        # Now evaluate any deferred expressions.
        for expression in self.deferredEvals:
            self.scriptRequest('eval', self.dom, value = expression,
                               needsResponse = False)
        self.deferredEvals = []

    def setP3DFilename(self, p3dFilename, tokens = [],
                       instanceId = None):
        # One day we will have support for multiple instances within a
        # Python session.  Against that day, we save the instance ID
        # for this instance.
        self.instanceId = instanceId

        self.tokens = tokens
        self.tokenDict = dict(tokens)

        # Tell the browser that Python is up and running, and ready to
        # respond to queries.
        self.notifyRequest('onpythonload')

        # Now go load the applet.
        fname = Filename.fromOsSpecific(p3dFilename)
        vfs = VirtualFileSystem.getGlobalPtr()

        if not vfs.exists(fname):
            raise ArgumentError, "No such file: %s" % (p3dFilename)

        fname.makeAbsolute()
        mf = Multifile()
        if not mf.openRead(fname):
            raise ArgumentError, "Not a Panda Multifile: %s" % (p3dFilename)

        # Now load the p3dInfo file.
        self.p3dInfo = None
        self.p3dPackage = None
        i = mf.findSubfile('p3d_info.xml')
        if i >= 0:
            stream = mf.openReadSubfile(i)
            self.p3dInfo = readXmlStream(stream)
            mf.closeReadSubfile(stream)
        if self.p3dInfo:
            self.p3dPackage = self.p3dInfo.FirstChildElement('package')

        if self.p3dPackage:
            fullDiskAccess = self.p3dPackage.Attribute('full_disk_access')
            try:
                self.fullDiskAccess = int(fullDiskAccess or '')
            except ValueError:
                pass

        self.initPackedAppEnvironment()

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

        # Send this call to the main thread; don't call it directly.
        messenger.send('startIfReady', taskChain = 'default')

    def clearWindowPrc(self):
        """ Clears the windowPrc file that was created in a previous
        call to setupWindow(), if any. """
        
        if self.windowPrc:
            unloadPrcFile(self.windowPrc)
            self.windowPrc = None

    def setupWindow(self, windowType, x, y, width, height,
                    parent, subprocessWindow):
        """ Applies the indicated window parameters to the prc
        settings, for future windows; or applies them directly to the
        main window if the window has already been opened. """

        print "session %s, nextScriptId = %s" % (self.sessionId, self.nextScriptId)


        print "setupWindow %s, %s, %s, %s, %s, %s, %s" % (windowType, x, y, width, height, parent, subprocessWindow)

        if self.started and base.win:
            # If we've already got a window, this must be a
            # resize/reposition request.
            wp = WindowProperties()
            if x or y or windowType == 'embedded':
                wp.setOrigin(x, y)
            if width or height:
                wp.setSize(width, height)
            if subprocessWindow:
                wp.setSubprocessWindow(subprocessWindow)
            base.win.requestProperties(wp)
            return

        # If we haven't got a window already, start 'er up.  Apply the
        # requested setting to the prc file.

        if windowType == 'hidden':
            data = 'window-type none\n'
        else:
            data = 'window-type onscreen\n'

        if windowType == 'fullscreen':
            data += 'fullscreen 1\n'
        else:
            data += 'fullscreen 0\n'

        if windowType == 'embedded':
            data += 'parent-window-handle %s\nsubprocess-window %s\n' % (
                parent, subprocessWindow)
        else:
            data += 'parent-window-handle 0\nsubprocess-window \n'

        if x or y or windowType == 'embedded':
            data += 'win-origin %s %s\n' % (x, y)
        if width or height:
            data += 'win-size %s %s\n' % (width, height)

        self.clearWindowPrc()
        self.windowPrc = loadPrcFileData("setupWindow", data)

        self.gotWindow = True

        # Send this call to the main thread; don't call it directly.
        messenger.send('startIfReady', taskChain = 'default')

    def setRequestFunc(self, func):
        """ This method is called by the plugin at startup to supply a
        function that can be used to deliver requests upstream, to the
        plugin, and thereby to the browser. """
        self.requestFunc = func

    def sendRequest(self, request, *args):
        """ Delivers a request to the browser via self.requestFunc.
        This low-level function is not intended to be called directly
        by user code. """
        
        assert self.requestFunc
        return self.requestFunc(self.instanceId, request, args)

    def windowEvent(self, win):
        """ This method is called when we get a window event.  We
        listen for this to detect when the window has been
        successfully opened. """

        if not self.windowOpened:
            self.windowOpened = True

            # Now that the window is open, we don't need to keep those
            # prc settings around any more.
            self.clearWindowPrc()

            # Inform the plugin and browser.
            self.notifyRequest('onwindowopen')

    def notifyRequest(self, message):
        """ Delivers a notify request to the browser.  This is a "this
        happened" type notification; it also triggers some JavaScript
        code execution, if indicated in the HTML tags, and may also
        trigger some internal automatic actions.  (For instance, the
        plugin takes down the splash window when it sees the
        onwindowopen notification. """

        self.sendRequest('notify', message)

    def evalScript(self, expression, needsResponse = False):
        """ Evaluates an arbitrary JavaScript expression in the global
        DOM space.  This may be deferred if necessary if needsResponse
        is False and self.dom has not yet been assigned.  If
        needsResponse is true, this waits for the value and returns
        it, which means it cannot be deferred. """

        if not self.dom:
            # Defer the expression.
            assert not needsResponse
            self.deferredEvals.append(expression)
        else:
            # Evaluate it now.
            return self.scriptRequest('eval', self.dom, value = expression,
                                      needsResponse = needsResponse)
        
    def scriptRequest(self, operation, object, propertyName = '',
                      value = None, needsResponse = True):
        """ Issues a new script request to the browser.  This queries
        or modifies one of the browser's DOM properties.
        
        operation may be one of [ 'get_property', 'set_property',
        'call', 'evaluate' ].

        object is the browser object to manipulate, or the scope in
        which to evaluate the expression.

        propertyName is the name of the property to manipulate, if
        relevant (set to None for the default method name).

        value is the new value to assign to the property for
        set_property, or the parameter list for call, or the string
        expression for evaluate.

        If needsResponse is true, this method will block until the
        return value is received from the browser, and then it returns
        that value.  Otherwise, it returns None immediately, without
        waiting for the browser to process the request.
        """
        uniqueId = self.nextScriptId
        self.nextScriptId = (self.nextScriptId + 1) % 0xffffffff
        self.sendRequest('script', operation, object,
                         propertyName, value, needsResponse, uniqueId)

        if needsResponse:
            # Now wait for the response to come in.
            result = self.sendRequest('wait_script_response', uniqueId)
            return result

    def dropObject(self, objectId):
        """ Inform the parent process that we no longer have an
        interest in the P3D_object corresponding to the indicated
        objectId. """

        self.sendRequest('drop_p3dobj', objectId)

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
    taskMgr.run()
