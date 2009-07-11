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
from pandac.PandaModules import VirtualFileSystem, Filename, Multifile, loadPrcFileData, unloadPrcFile, getModelPath, HTTPClient, Thread, WindowProperties
from direct.stdpy import file
from direct.task.TaskManagerGlobal import taskMgr
from direct.showbase import AppRunnerGlobal
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
        
        self.packedAppEnvironmentInitialized = False
        self.gotWindow = False
        self.gotP3DFilename = False
        self.started = False
        self.windowOpened = False
        self.windowPrc = None

        # Store this Undefined instance where the application can easily
        # get to it.
        self.Undefined = Undefined

        # This is per session.
        self.nextScriptId = 0

        # TODO: we need one of these per instance, not per session.
        self.instanceId = None

        # The attributes of this object will be exposed as attributes
        # of the plugin instance in the DOM.
        self.attributes = ScriptAttributes()

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
        if not p3dFilename:
            # If we didn't get a literal filename, we have to download it
            # from the URL.  TODO: make this a smarter temporary filename?
            fname = Filename.temporary('', 'p3d_')
            fname.setExtension('p3d')
            p3dFilename = fname.toOsSpecific()
            src = self.tokenDict.get('src', None)
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
        print "setupWindow %s, %s, %s, %s, %s, %s" % (windowType, x, y, width, height, parent)
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

        if x or y or windowType == 'embedded':
            data += 'win-origin %s %s\n' % (x, y)
        if width or height:
            data += 'win-size %s %s\n' % (width, height)

        if self.windowPrc:
            unloadPrcFile(self.windowPrc)
        self.windowPrc = loadPrcFileData("setupWindow", data)

        if self.started and base.win:
            # If we've already got a window, this must be a
            # resize/reposition request.
            wp = WindowProperties()
            if x or y or windowType == 'embedded':
                wp.setOrigin(x, y)
            if width or height:
                wp.setSize(width, height)
            base.win.requestProperties(wp)

        else:
            # If we haven't got a window already, start 'er up.
            self.gotWindow = True
            self.startIfReady()

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
            self.notifyRequest('onwindowopen')
            self.windowOpened = True

    def notifyRequest(self, message):
        """ Delivers a notify request to the browser.  This is a "this
        happened" type notification; it optionally triggers some
        JavaScript code execution, and may also trigger some internal
        automatic actions.  (For instance, the plugin takes down the
        splash window when it sees the onwindowopen notification. """

        self.sendRequest('notify', message)

        # Now process any JavaScript that might be waiting for the
        # event as well.  These are the JavaScript expressions that
        # were specified in the HTML embed or object tag.
        expression = self.tokenDict.get(message)
        if expression:
            self.evalScript(expression)

    def evalScript(self, expression, needsResponse = False):
        """ Evaluates an arbitrary JavaScript expression in the global
        DOM space.  This may be deferred if necessary if self.dom has
        not yet been assigned.  If needsResponse is true, this waits
        for the value and returns it, which means it may not be
        deferred. """

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
        self.nextScriptId += 1
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

class UndefinedObject:
    """ This is a special object that is returned by the browser to
    represent a NULL pointer, typically the return value for a failed
    operation.  It has no attributes, similar to None. """

    def __nonzero__(self):
        return False

    def __str__(self):
        return "Undefined"

# In fact, we normally always return this precise instance of the
# UndefinedObject.
Undefined = UndefinedObject()

class BrowserObject:
    """ This class provides the Python wrapper around some object that
    actually exists in the plugin host's namespace, e.g. a JavaScript
    or DOM object. """

    def __init__(self, runner, objectId):
        self.__dict__['_BrowserObject__runner'] = runner
        self.__dict__['_BrowserObject__objectId'] = objectId

        # This element is filled in by __getattr__; it connects
        # the object to its parent.
        self.__dict__['_BrowserObject__boundMethod'] = (None, None)

    def __del__(self):
        # When the BrowserObject destructs, tell the parent process it
        # doesn't need to keep around its corresponding P3D_object any
        # more.
        self.__runner.dropObject(self.__objectId)

    def __str__(self):
        parentObj, attribName = self.__boundMethod
        if parentObj:
            # Format it from its parent.
            return "%s.%s" % (parentObj, attribName)
        else:
            # Format it directly.
            return "BrowserObject(%s)" % (self.__objectId)

    def __nonzero__(self):
        return True

    def __call__(self, *args):
        parentObj, attribName = self.__boundMethod
        if parentObj:
            # Call it as a method.
            needsResponse = True
            if parentObj is self.__runner.dom and attribName == 'alert':
                # As a special hack, we don't wait for the return
                # value from the alert() call, since this is a
                # blocking call, and waiting for this could cause
                # problems.
                needsResponse = False

            if parentObj is self.__runner.dom and attribName == 'eval' and len(args) == 1 and isinstance(args[0], types.StringTypes):
                # As another special hack, we make dom.eval() a
                # special case, and map it directly into an eval()
                # call.  If the string begins with 'void ', we further
                # assume we're not waiting for a response.
                if args[0].startswith('void '):
                    needsResponse = False
                result = self.__runner.scriptRequest('eval', parentObj, value = args[0], needsResponse = needsResponse)
            else:
                # This is a normal method call.
                result = self.__runner.scriptRequest('call', parentObj, propertyName = attribName, value = args, needsResponse = needsResponse)
        else:
            # Call it as a plain function.
            result = self.__runner.scriptRequest('call', self, value = args)

        return result

    def __getattr__(self, name):
        """ Remaps attempts to query an attribute, as in obj.attr,
        into the appropriate calls to query the actual browser object
        under the hood.  """

        try:
            value = self.__runner.scriptRequest('get_property', self,
                                                propertyName = name)
        except EnvironmentError:
            # Failed to retrieve the attribute.
            raise AttributeError(name)

        if isinstance(value, BrowserObject):
            # Fill in the parent object association, so __call__ can
            # properly call a method.  (Javascript needs to know the
            # method container at the time of the call, and doesn't
            # store it on the function object.)
            value.__dict__['_BrowserObject__boundMethod'] = (self, name)

        return value

    def __setattr__(self, name, value):
        if name in self.__dict__:
            self.__dict__[name] = value
            return

        result = self.__runner.scriptRequest('set_property', self,
                                             propertyName = name,
                                             value = value)
        if not result:
            raise AttributeError(name)

    def __delattr__(self, name):
        if name in self.__dict__:
            del self.__dict__[name]
            return

        result = self.__runner.scriptRequest('del_property', self,
                                             propertyName = name)
        if not result:
            raise AttributeError(name)

    def __getitem__(self, key):
        """ Remaps attempts to query an attribute, as in obj['attr'],
        into the appropriate calls to query the actual browser object
        under the hood.  Following the JavaScript convention, we treat
        obj['attr'] almost the same as obj.attr. """

        try:
            value = self.__runner.scriptRequest('get_property', self,
                                                propertyName = str(key))
        except EnvironmentError:
            # Failed to retrieve the property.  We return IndexError
            # for numeric keys so we can properly support Python's
            # iterators, but we return KeyError for string keys to
            # emulate mapping objects.
            if isinstance(key, types.StringTypes):
                raise KeyError(key)
            else:
                raise IndexError(key)

        return value

    def __setitem__(self, key, value):
        result = self.__runner.scriptRequest('set_property', self,
                                             propertyName = str(key),
                                             value = value)
        if not result:
            if isinstance(key, types.StringTypes):
                raise KeyError(key)
            else:
                raise IndexError(key)

    def __delitem__(self, key):
        result = self.__runner.scriptRequest('del_property', self,
                                             propertyName = str(key))
        if not result:
            if isinstance(key, types.StringTypes):
                raise KeyError(key)
            else:
                raise IndexError(key)

if __name__ == '__main__':
    runner = AppRunner()
    runner.gotWindow = True
    try:
        runner.setP3DFilename(*runner.parseSysArgs())
    except ArgumentError, e:
        print e.args[0]
        sys.exit(1)
    run()
