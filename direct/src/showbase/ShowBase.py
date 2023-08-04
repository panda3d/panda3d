""" This module contains `.ShowBase`, an application framework responsible
for opening a graphical display, setting up input devices and creating
the scene graph.

The simplest way to open a ShowBase instance is to execute this code:

.. code-block:: python

   from direct.showbase.ShowBase import ShowBase

   base = ShowBase()
   base.run()

A common approach is to create your own subclass inheriting from ShowBase.

Built-in global variables
-------------------------

Some key variables used in all Panda3D scripts are actually attributes of the
ShowBase instance.  When creating an instance of this class, it will write many
of these variables to the built-in scope of the Python interpreter, so that
they are accessible to any Python module, without the need for extra imports.
For example, the ShowBase instance itself is accessible anywhere through the
:data:`~builtins.base` variable.

While these are handy for prototyping, we do not recommend using them in bigger
projects, as it can make the code confusing to read to other Python developers,
to whom it may not be obvious where these variables are originating.

Refer to the :mod:`builtins` page for a listing of the variables written to the
built-in scope.

"""

__all__ = ['ShowBase', 'WindowControls']

# This module redefines the builtin import function with one
# that prints out every import it does in a hierarchical form
# Annoying and very noisy, but sometimes useful
#import VerboseImport

from panda3d.core import *
from panda3d.direct import throw_new_frame, init_app_for_gui
from panda3d.direct import storeAccessibilityShortcutKeys, allowAccessibilityShortcutKeys
from . import DConfig

# Register the extension methods for NodePath.
from direct.extensions_native import NodePath_extensions

# This needs to be available early for DirectGUI imports
import sys
if sys.version_info >= (3, 0):
    import builtins
else:
    import __builtin__ as builtins
builtins.config = DConfig

from direct.directnotify.DirectNotifyGlobal import directNotify, giveNotify
from .MessengerGlobal import messenger
from .BulletinBoardGlobal import bulletinBoard
from direct.task.TaskManagerGlobal import taskMgr
from .JobManagerGlobal import jobMgr
from .EventManagerGlobal import eventMgr
#from PythonUtil import *
from direct.interval import IntervalManager
from direct.showbase.BufferViewer import BufferViewer
from direct.task import Task
from . import Loader
import time
import atexit
import importlib
from direct.showbase import ExceptionVarDump
from . import DirectObject
from . import SfxPlayer
if __debug__:
    from direct.showbase import GarbageReport
    from direct.directutil import DeltaProfiler
    from . import OnScreenDebug
from . import AppRunnerGlobal

@atexit.register
def exitfunc():
    if getattr(builtins, 'base', None) is not None:
        builtins.base.destroy()

# Now ShowBase is a DirectObject.  We need this so ShowBase can hang
# hooks on messages, particularly on window-event.  This doesn't
# *seem* to cause anyone any problems.
class ShowBase(DirectObject.DirectObject):

    #: The deprecated `.DConfig` interface for accessing config variables.
    config = DConfig
    notify = directNotify.newCategory("ShowBase")

    def __init__(self, fStartDirect=True, windowType=None):
        """Opens a window, sets up a 3-D and several 2-D scene graphs, and
        everything else needed to render the scene graph to the window.

        To prevent a window from being opened, set windowType to the string
        'none' (or 'offscreen' to create an offscreen buffer).  If this is not
        specified, the default value is taken from the 'window-type'
        configuration variable.

        This constructor will add various things to the Python builtins scope,
        including this instance itself (under the name ``base``).
        """

        #: Set if the want-dev Config.prc variable is enabled.  By default, it
        #: is set to True except when using Python with the -O flag.
        self.__dev__ = self.config.GetBool('want-dev', __debug__)
        builtins.__dev__ = self.__dev__

        logStackDump = (self.config.GetBool('log-stack-dump', False) or
                        self.config.GetBool('client-log-stack-dump', False))
        uploadStackDump = self.config.GetBool('upload-stack-dump', False)
        if logStackDump or uploadStackDump:
            ExceptionVarDump.install(logStackDump, uploadStackDump)

        if __debug__:
            self.__autoGarbageLogging = self.__dev__ and self.config.GetBool('auto-garbage-logging', False)

        #: The directory containing the main Python file of this application.
        self.mainDir = ExecutionEnvironment.getEnvironmentVariable("MAIN_DIR")
        self.main_dir = self.mainDir

        #: This contains the global appRunner instance, as imported from
        #: `.AppRunnerGlobal`.  This will be None if we are not running in the
        #: runtime environment (ie. from a .p3d file).  Deprecated.
        self.appRunner = AppRunnerGlobal.appRunner
        self.app_runner = self.appRunner

        #debug running multiplier
        self.debugRunningMultiplier = 4

        # [gjeon] to disable sticky keys
        if self.config.GetBool('disable-sticky-keys', 0):
            storeAccessibilityShortcutKeys()
            allowAccessibilityShortcutKeys(False)

        self.printEnvDebugInfo()
        vfs = VirtualFileSystem.getGlobalPtr()

        self.nextWindowIndex = 1
        self.__directStarted = False
        self.__deadInputs = 0

        # Store dconfig variables
        self.sfxActive = self.config.GetBool('audio-sfx-active', 1)
        self.musicActive = self.config.GetBool('audio-music-active', 1)
        self.wantFog = self.config.GetBool('want-fog', 1)
        self.wantRender2dp = self.config.GetBool('want-render2dp', 1)

        self.screenshotExtension = self.config.GetString('screenshot-extension', 'jpg')
        self.musicManager = None
        self.musicManagerIsValid = None
        self.sfxManagerList = []
        self.sfxManagerIsValidList = []

        self.wantStats = self.config.GetBool('want-pstats', 0)
        self.wantTk = False
        self.wantWx = False
        self.wantDirect = False

        #: Fill this in with a function to invoke when the user "exits"
        #: the program by closing the main window.
        self.exitFunc = None

        #: Add final-exit callbacks to this list.  These will be called
        #: when sys.exit() is called, after Panda has unloaded, and
        #: just before Python is about to shut down.
        self.finalExitCallbacks = []

        # Set up the TaskManager to reset the PStats clock back
        # whenever we resume from a pause.  This callback function is
        # a little hacky, but we can't call it directly from within
        # the TaskManager because he doesn't know about PStats (and
        # has to run before libpanda is even loaded).
        taskMgr.resumeFunc = PStatClient.resumeAfterPause

        if self.__dev__:
            self.__setupProfile()

        # If the aspect ratio is 0 or None, it means to infer the
        # aspect ratio from the window size.
        # If you need to know the actual aspect ratio call base.getAspectRatio()
        self.__configAspectRatio = ConfigVariableDouble('aspect-ratio', 0).getValue()
        # This variable is used to see if the aspect ratio has changed when
        # we get a window-event.
        self.__oldAspectRatio = None

        #: This is set to the value of the window-type config variable, but may
        #: optionally be overridden in the Showbase constructor.  Should either
        #: be 'onscreen' (the default), 'offscreen' or 'none'.
        self.windowType = windowType
        if self.windowType is None:
            self.windowType = self.config.GetString('window-type', 'onscreen')
        self.requireWindow = self.config.GetBool('require-window', 1)

        #: This is the main, or only window; see `winList` for a list of *all* windows.
        self.win = None
        self.frameRateMeter = None
        self.sceneGraphAnalyzerMeter = None
        #: A list of all windows opened via `openWindow()`.
        self.winList = []
        self.winControls = []
        self.mainWinMinimized = 0
        self.mainWinForeground = 0
        #: Contains the :class:`~panda3d.core.GraphicsPipe` object created by
        #: `makeDefaultPipe()`.
        self.pipe = None
        #: The full list of :class:`~panda3d.core.GraphicsPipe` objects,
        #: including any auxiliary pipes.  Filled by `makeAllPipes()`.
        self.pipeList = []
        self.mouse2cam = None
        self.buttonThrowers = None
        self.mouseWatcher = None
        #: The :class:`~panda3d.core.MouseWatcher` object, created by
        #: `setupMouse()`.
        self.mouseWatcherNode = None
        self.pointerWatcherNodes = None
        self.mouseInterface = None
        self.drive = None
        self.trackball = None
        self.texmem = None
        self.showVertices = None
        self.deviceButtonThrowers = []

        #: This is a :class:`~panda3d.core.NodePath` pointing to the
        #: :class:`~panda3d.core.Camera` object set up for the 3D scene.
        #: Usually a child of `camera`.
        self.cam = None
        #: Same as `cam`, but for the 2D scene graph.
        self.cam2d = None
        #: Same as `cam2d`, but for the 2D overlay scene graph.
        self.cam2dp = None

        #: This is the :class:`~panda3d.core.NodePath` that should be used to
        #: manipulate the camera.  It points at the node to which the default
        #: camera (`cam`, `camNode`) is attached.
        self.camera = None
        #: Same as `camera`, but for the 2D scene graph.  Parent of `cam2d`.
        self.camera2d = None
        #: Same as `camera2d`, but for the 2D overlay scene graph.  Parent of
        #: `cam2dp`.
        self.camera2dp = None

        #: A list of all cameras created with `makeCamera()`, including `cam`.
        self.camList = []
        #: Convenience accessor for base.cam.node(), containing a
        #: :class:`~panda3d.core.Camera` object.
        self.camNode = None
        #: Convenience accessor for base.camNode.get_lens(), containing a
        #: :class:`~panda3d.core.Lens` object.
        self.camLens = None
        self.camFrustumVis = None
        self.direct = None
        #: This is used to store the wx.Application object used when want-wx is
        #: set or `startWx()` is called.
        self.wxApp = None
        self.wxAppCreated = False
        self.tkRoot = None
        self.tkRootCreated = False

        # This is used for syncing multiple PCs in a distributed cluster
        try:
            # Has the cluster sync variable been set externally?
            self.clusterSyncFlag = clusterSyncFlag
        except NameError:
            # Has the clusterSyncFlag been set via a config variable
            self.clusterSyncFlag = self.config.GetBool('cluster-sync', 0)

        # We've already created aspect2d in ShowBaseGlobal, for the
        # benefit of creating DirectGui elements before ShowBase.
        from . import ShowBaseGlobal
        self.hidden = ShowBaseGlobal.hidden

        #: The global :class:`~panda3d.core.GraphicsEngine`, as returned by
        #: GraphicsEngine.getGlobalPtr()
        self.graphicsEngine = GraphicsEngine.getGlobalPtr()
        self.graphics_engine = self.graphicsEngine
        self.setupRender()
        self.setupRender2d()
        self.setupDataGraph()

        if self.wantRender2dp:
            self.setupRender2dp()

        #: A placeholder for a :class:`~panda3d.core.CollisionTraverser`.  If
        #: someone stores a CollisionTraverser pointer here, ShowBase will
        #: traverse it automatically in the collisionLoop task, so you won't
        #: need to call :meth:`~panda3d.core.CollisionTraverser.traverse()`
        #: yourself every frame.
        self.cTrav = 0
        self.shadowTrav = 0
        self.cTravStack = Stack()
        # Ditto for an AppTraverser.
        self.appTrav = 0

        # This is the DataGraph traverser, which we might as well
        # create now.
        self.dgTrav = DataGraphTraverser()

        # Maybe create a RecorderController to record and/or play back
        # the user session.
        self.recorder = None
        playbackSession = self.config.GetString('playback-session', '')
        recordSession = self.config.GetString('record-session', '')
        if playbackSession:
            self.recorder = RecorderController()
            self.recorder.beginPlayback(Filename.fromOsSpecific(playbackSession))
        elif recordSession:
            self.recorder = RecorderController()
            self.recorder.beginRecord(Filename.fromOsSpecific(recordSession))

        if self.recorder:
            # If we're either playing back or recording, pass the
            # random seed into the system so each session will have
            # the same random seed.
            import random #, whrandom

            seed = self.recorder.getRandomSeed()
            random.seed(seed)
            #whrandom.seed(seed & 0xff, (seed >> 8) & 0xff, (seed >> 16) & 0xff)

        # For some reason, wx needs to be initialized before the graphics window
        if sys.platform == "darwin":
            if self.config.GetBool("want-wx", 0):
                wx = importlib.import_module('wx')
                self.wxApp = wx.App()

            # Same goes for Tk, which uses a conflicting NSApplication
            if self.config.GetBool("want-tk", 0):
                Pmw = importlib.import_module('Pmw')
                self.tkRoot = Pmw.initialise()

        # Open the default rendering window.
        if self.windowType != 'none':
            props = WindowProperties.getDefault()
            if (self.config.GetBool('read-raw-mice', 0)):
                props.setRawMice(1)
            self.openDefaultWindow(startDirect = False, props=props)

        # The default is trackball mode, which is more convenient for
        # ad-hoc development in Python using ShowBase.  Applications
        # can explicitly call base.useDrive() if they prefer a drive
        # interface.
        self.mouseInterface = self.trackball
        self.useTrackball()

        #: `.Loader.Loader` object.
        self.loader = Loader.Loader(self)
        self.graphicsEngine.setDefaultLoader(self.loader.loader)

        #: The global event manager, as imported from `.EventManagerGlobal`.
        self.eventMgr = eventMgr
        #: The global messenger, as imported from `.MessengerGlobal`.
        self.messenger = messenger
        #: The global bulletin board, as imported from `.BulletinBoardGlobal`.
        self.bboard = bulletinBoard
        #: The global task manager, as imported from `.TaskManagerGlobal`.
        self.taskMgr = taskMgr
        self.task_mgr = taskMgr
        #: The global job manager, as imported from `.JobManagerGlobal`.
        self.jobMgr = jobMgr

        #: If `enableParticles()` has been called, this is the particle manager
        #: as imported from :mod:`direct.particles.ParticleManagerGlobal`.
        self.particleMgr = None
        self.particleMgrEnabled = 0

        #: If `enableParticles()` has been called, this is the physics manager
        #: as imported from :mod:`direct.showbase.PhysicsManagerGlobal`.
        self.physicsMgr = None
        self.physicsMgrEnabled = 0
        self.physicsMgrAngular = 0

        #: This is the global :class:`~panda3d.core.InputDeviceManager`, which
        #: keeps track of connected input devices.
        self.devices = InputDeviceManager.getGlobalPtr()
        self.__inputDeviceNodes = {}

        self.createStats()

        self.AppHasAudioFocus = 1

        # Get a pointer to Panda's global ClockObject, used for
        # synchronizing events between Python and C.
        clock = ClockObject.getGlobalClock()

        #: This is the global :class:`~panda3d.core.ClockObject`.
        self.clock = clock

        # Since we have already started up a TaskManager, and probably
        # a number of tasks; and since the TaskManager had to use the
        # TrueClock to tell time until this moment, make sure the
        # globalClock object is exactly in sync with the TrueClock.
        trueClock = TrueClock.getGlobalPtr()
        clock.setRealTime(trueClock.getShortTime())
        clock.tick()

        # Now we can make the TaskManager start using the new clock.
        taskMgr.globalClock = clock

        # client CPU affinity is determined by, in order:
        # - client-cpu-affinity-mask config
        # - pcalt-# (# is CPU number, 0-based)
        # - client-cpu-affinity config
        # - auto-single-cpu-affinity config
        affinityMask = self.config.GetInt('client-cpu-affinity-mask', -1)
        if affinityMask != -1:
            TrueClock.getGlobalPtr().setCpuAffinity(affinityMask)
        else:
            # this is useful on machines that perform better with each process
            # assigned to a single CPU
            autoAffinity = self.config.GetBool('auto-single-cpu-affinity', 0)
            affinity = None
            if autoAffinity and hasattr(builtins, 'clientIndex'):
                affinity = abs(int(builtins.clientIndex))
            else:
                affinity = self.config.GetInt('client-cpu-affinity', -1)
            if (affinity in (None, -1)) and autoAffinity:
                affinity = 0
            if affinity not in (None, -1):
                # Windows XP supports a 32-bit affinity mask
                TrueClock.getGlobalPtr().setCpuAffinity(1 << (affinity % 32))

        # Make sure we're not making more than one ShowBase.
        if hasattr(builtins, 'base'):
            raise Exception("Attempt to spawn multiple ShowBase instances!")

        # DO NOT ADD TO THIS LIST.  We're trying to phase out the use of
        # built-in variables by ShowBase.  Use a Global module if necessary.
        builtins.base = self
        builtins.render2d = self.render2d
        builtins.aspect2d = self.aspect2d
        builtins.pixel2d = self.pixel2d
        builtins.render = self.render
        builtins.hidden = self.hidden
        builtins.camera = self.camera
        builtins.loader = self.loader
        builtins.taskMgr = self.taskMgr
        builtins.jobMgr = self.jobMgr
        builtins.eventMgr = self.eventMgr
        builtins.messenger = self.messenger
        builtins.bboard = self.bboard
        # Config needs to be defined before ShowBase is constructed
        #builtins.config = self.config
        builtins.ostream = Notify.out()
        builtins.directNotify = directNotify
        builtins.giveNotify = giveNotify
        builtins.globalClock = clock
        builtins.vfs = vfs
        builtins.cpMgr = ConfigPageManager.getGlobalPtr()
        builtins.cvMgr = ConfigVariableManager.getGlobalPtr()
        builtins.pandaSystem = PandaSystem.getGlobalPtr()
        builtins.wantUberdog = self.config.GetBool('want-uberdog', 1)
        if __debug__:
            builtins.deltaProfiler = DeltaProfiler.DeltaProfiler("ShowBase")
            self.onScreenDebug = OnScreenDebug.OnScreenDebug()
            builtins.onScreenDebug = self.onScreenDebug

        if self.wantRender2dp:
            builtins.render2dp = self.render2dp
            builtins.aspect2dp = self.aspect2dp
            builtins.pixel2dp = self.pixel2dp

        # Now add this instance to the ShowBaseGlobal module scope.
        from . import ShowBaseGlobal
        builtins.run = ShowBaseGlobal.run
        ShowBaseGlobal.base = self
        ShowBaseGlobal.__dev__ = self.__dev__

        if self.__dev__:
            ShowBase.notify.debug('__dev__ == %s' % self.__dev__)
        else:
            ShowBase.notify.info('__dev__ == %s' % self.__dev__)

        self.createBaseAudioManagers()

        if self.__dev__ and self.config.GetBool('track-gui-items', False):
            # dict of guiId to gui item, for tracking down leaks
            if not hasattr(ShowBase, 'guiItems'):
                ShowBase.guiItems = {}

        # optionally restore the default gui sounds from 1.7.2 and earlier
        if ConfigVariableBool('orig-gui-sounds', False).getValue():
            from direct.gui import DirectGuiGlobals as DGG
            DGG.setDefaultClickSound(self.loader.loadSfx("audio/sfx/GUI_click.wav"))
            DGG.setDefaultRolloverSound(self.loader.loadSfx("audio/sfx/GUI_rollover.wav"))

        # Now hang a hook on the window-event from Panda.  This allows
        # us to detect when the user resizes, minimizes, or closes the
        # main window.
        self.__prevWindowProperties = None
        self.accept('window-event', self.windowEvent)

        # Transition effects (fade, iris, etc)
        from . import Transitions

        #: `.Transitions.Transitions` object.
        self.transitions = Transitions.Transitions(self.loader)

        if self.win:
            # Setup the window controls - handy for multiwindow applications
            self.setupWindowControls()

        # Client sleep
        sleepTime = self.config.GetFloat('client-sleep', 0.0)
        self.clientSleep = 0.0
        self.setSleep(sleepTime)

        # Extra sleep for running 4+ clients on a single machine
        # adds a sleep right after the main render in igloop
        # tends to even out the frame rate and keeps it from going
        # to zero in the out of focus windows
        if self.config.GetBool('multi-sleep', 0):
            self.multiClientSleep = 1
        else:
            self.multiClientSleep = 0

        #: Utility for viewing offscreen buffers, see :mod:`.BufferViewer`.
        self.bufferViewer = BufferViewer(self.win, self.render2dp if self.wantRender2dp else self.render2d)

        if self.windowType != 'none':
            if fStartDirect: # [gjeon] if this is False let them start direct manually
                self.__doStartDirect()

            if self.config.GetBool('show-tex-mem', False):
                if not self.texmem or self.texmem.cleanedUp:
                    self.toggleTexMem()

        taskMgr.finalInit()

        # Start IGLOOP
        self.restart()

    # add a collision traverser via pushCTrav and remove it via popCTrav
    # that way the owner of the new cTrav doesn't need to hold onto the
    # previous one in order to put it back
    def pushCTrav(self, cTrav):
        self.cTravStack.push(self.cTrav)
        self.cTrav = cTrav
    def popCTrav(self):
        self.cTrav = self.cTravStack.pop()

    def __setupProfile(self):
        """ Sets up the Python profiler, if available, according to
        some Panda config settings. """

        try:
            profile = importlib.import_module('profile')
            pstats = importlib.import_module('pstats')
        except ImportError:
            return

        profile.Profile.bias = float(self.config.GetString("profile-bias","0"))

        def f8(x):
            return ("%" + "8.%df" % self.config.GetInt("profile-decimals", 3)) % x
        pstats.f8 = f8

    # temp; see ToonBase.py
    def getExitErrorCode(self):
        return 0

    def printEnvDebugInfo(self):
        """Print some information about the environment that we are running
        in.  Stuff like the model paths and other paths.  Feel free to
        add stuff to this.
        """
        if self.config.GetBool('want-env-debug-info', 0):
           print("\n\nEnvironment Debug Info {")
           print("* model path:")
           print(getModelPath())
           #print "* dna path:"
           #print getDnaPath()
           print("}")

    def destroy(self):
        """ Call this function to destroy the ShowBase and stop all
        its tasks, freeing all of the Panda resources.  Normally, you
        should not need to call it explicitly, as it is bound to the
        exitfunc and will be called at application exit time
        automatically.

        This function is designed to be safe to call multiple times."""

        for cb in self.finalExitCallbacks[:]:
            cb()

        # Remove the built-in base reference
        if getattr(builtins, 'base', None) is self:
            del builtins.run
            del builtins.base
            del builtins.loader
            del builtins.taskMgr
            ShowBaseGlobal = sys.modules.get('direct.showbase.ShowBaseGlobal', None)
            if ShowBaseGlobal:
                del ShowBaseGlobal.base

        self.aspect2d.node().removeAllChildren()
        self.render2d.node().removeAllChildren()
        self.aspect2d.reparent_to(self.render2d)

        # [gjeon] restore sticky key settings
        if self.config.GetBool('disable-sticky-keys', 0):
            allowAccessibilityShortcutKeys(True)

        self.ignoreAll()
        self.shutdown()

        if getattr(self, 'musicManager', None):
            self.musicManager.shutdown()
            self.musicManager = None
            for sfxManager in self.sfxManagerList:
                sfxManager.shutdown()
            self.sfxManagerList = []
        if getattr(self, 'loader', None):
            self.loader.destroy()
            self.loader = None
        if getattr(self, 'graphicsEngine', None):
            self.graphicsEngine.removeAllWindows()

        try:
            self.direct.panel.destroy()
        except:
            pass

        if hasattr(self, 'win'):
            del self.win
            del self.winList
            del self.pipe

    def makeDefaultPipe(self, printPipeTypes = None):
        """
        Creates the default GraphicsPipe, which will be used to make
        windows unless otherwise specified.
        """
        assert self.pipe is None

        if printPipeTypes is None:
            # When the user didn't specify an explicit setting, take the value
            # from the config variable. We could just omit the parameter, however
            # this way we can keep backward compatibility.
            printPipeTypes = ConfigVariableBool("print-pipe-types", True)

        selection = GraphicsPipeSelection.getGlobalPtr()
        if printPipeTypes:
            selection.printPipeTypes()
        self.pipe = selection.makeDefaultPipe()
        if not self.pipe:
            self.notify.error(
                "No graphics pipe is available!\n"
                "Your Config.prc file must name at least one valid panda display\n"
                "library via load-display or aux-display.")

        self.notify.info("Default graphics pipe is %s (%s)." % (
            self.pipe.getType().getName(), self.pipe.getInterfaceName()))
        self.pipeList.append(self.pipe)

    def makeModulePipe(self, moduleName):
        """
        Returns a GraphicsPipe from the indicated module,
        e.g. 'pandagl' or 'pandadx9'.  Does not affect base.pipe or
        base.pipeList.

        :rtype: panda3d.core.GraphicsPipe
        """

        selection = GraphicsPipeSelection.getGlobalPtr()
        return selection.makeModulePipe(moduleName)

    def makeAllPipes(self):
        """
        Creates all GraphicsPipes that the system knows about and fill up
        `pipeList` with them.
        """
        selection = GraphicsPipeSelection.getGlobalPtr()
        selection.loadAuxModules()

        # First, we should make sure the default pipe exists.
        if self.pipe is None:
            self.makeDefaultPipe()

        # Now go through the list of known pipes, and make each one if
        # we don't have one already.
        numPipeTypes = selection.getNumPipeTypes()
        for i in range(numPipeTypes):
            pipeType = selection.getPipeType(i)

            # Do we already have a pipe of this type on the list?
            # This operation is n-squared, but presumably there won't
            # be more than a handful of pipe types, so who cares.
            already = 0
            for pipe in self.pipeList:
                if pipe.getType() == pipeType:
                    already = 1

            if not already:
                pipe = selection.makePipe(pipeType)
                if pipe:
                    self.notify.info("Got aux graphics pipe %s (%s)." % (
                        pipe.getType().getName(), pipe.getInterfaceName()))
                    self.pipeList.append(pipe)
                else:
                    self.notify.info("Could not make graphics pipe %s." % (
                        pipeType.getName()))

    def openWindow(self, props = None, fbprops = None, pipe = None, gsg = None,
                   host = None, type = None, name = None, size = None,
                   aspectRatio = None, makeCamera = True, keepCamera = False,
                   scene = None, stereo = None, unexposedDraw = None,
                   callbackWindowDict = None, requireWindow = None):
        """
        Creates a window and adds it to the list of windows that are
        to be updated every frame.

        :param props: the :class:`~panda3d.core.WindowProperties` that
                      describes the window.

        :param fbprops: the :class:`~panda3d.core.FrameBufferProperties`
                        indicating the requested framebuffer properties.

        :param type: Either 'onscreen', 'offscreen', or 'none'.

        :param keepCamera: If True, the existing base.cam is set up to
                           render into the new window.

        :param makeCamera: If True (and keepCamera is False), a new camera is
                           set up to render into the new window.

        :param unexposedDraw: If not None, it specifies the initial value
                              of :meth:`~panda3d.core.GraphicsWindow.setUnexposedDraw()`.

        :param callbackWindowDict: If not None, a
                                   :class:`~panda3d.core.CallbackGraphicsWindow`
                                   is created instead, which allows the caller
                                   to create the actual window with its own
                                   OpenGL context, and direct Panda's rendering
                                   into that window.

        :param requireWindow: If True, the function should raise an exception
                              if the window fails to open correctly.

        :rtype: panda3d.core.GraphicsWindow
        """

        # Save this lambda here for convenience; we'll use it to call
        # down to the underlying _doOpenWindow() with all of the above
        # parameters.
        func = lambda : self._doOpenWindow(
            props = props, fbprops = fbprops, pipe = pipe, gsg = gsg,
            host = host, type = type, name = name, size = size,
            aspectRatio = aspectRatio, makeCamera = makeCamera,
            keepCamera = keepCamera, scene = scene, stereo = stereo,
            unexposedDraw = unexposedDraw,
            callbackWindowDict = callbackWindowDict)

        if self.win:
            # If we've already opened a window before, this is just a
            # pass-through to _doOpenWindow().
            win = func()
            self.graphicsEngine.openWindows()
            return win

        if type is None:
            type = self.windowType
        if requireWindow is None:
            requireWindow = self.requireWindow

        win = func()

        # Give the window a chance to truly open.
        self.graphicsEngine.openWindows()
        if win is not None and not win.isValid():
            self.notify.info("Window did not open, removing.")
            self.closeWindow(win)
            win = None

        if win is None and pipe is None:
            # Try a little harder if the window wouldn't open.
            self.makeAllPipes()
            try:
                self.pipeList.remove(self.pipe)
            except ValueError:
                pass
            while self.win is None and self.pipeList:
                self.pipe = self.pipeList[0]
                self.notify.info("Trying pipe type %s (%s)" % (
                    self.pipe.getType(), self.pipe.getInterfaceName()))
                win = func()

                self.graphicsEngine.openWindows()
                if win is not None and not win.isValid():
                    self.notify.info("Window did not open, removing.")
                    self.closeWindow(win)
                    win = None
                if win is None:
                    self.pipeList.remove(self.pipe)

        if win is None:
            self.notify.warning("Unable to open '%s' window." % (type))
            if requireWindow:
                # Unless require-window is set to false, it is an
                # error not to open a window.
                raise Exception('Could not open window.')
        else:
            self.notify.info("Successfully opened window of type %s (%s)" % (
                win.getType(), win.getPipe().getInterfaceName()))

        return win

    def _doOpenWindow(self, props = None, fbprops = None, pipe = None,
                      gsg = None, host = None, type = None, name = None,
                      size = None, aspectRatio = None,
                      makeCamera = True, keepCamera = False,
                      scene = None, stereo = None, unexposedDraw = None,
                      callbackWindowDict = None):
        if pipe is None:
            pipe = self.pipe

            if pipe is None:
                self.makeDefaultPipe()
                pipe = self.pipe

            if pipe is None:
                # We couldn't get a pipe.
                return None

        if isinstance(gsg, GraphicsOutput):
            # If the gsg is a window or buffer, it means to use the
            # GSG from that buffer.
            host = gsg
            gsg = gsg.getGsg()

        # If we are using DirectX, force a new GSG to be created,
        # since at the moment DirectX seems to misbehave if we do
        # not do this.  This will cause a delay while all textures
        # etc. are reloaded, so we should revisit this later if we
        # can fix the underlying bug in our DirectX support.
        if pipe.getType().getName().startswith('wdx'):
            gsg = None

        if type is None:
            type = self.windowType

        if props is None:
            props = WindowProperties.getDefault()

        if fbprops is None:
            fbprops = FrameBufferProperties.getDefault()

        if size is not None:
            # If we were given an explicit size, use it; otherwise,
            # the size from the properties is used.
            props = WindowProperties(props)
            props.setSize(size[0], size[1])

        if name is None:
            name = 'window%s' % (self.nextWindowIndex)
            self.nextWindowIndex += 1

        win = None

        flags = GraphicsPipe.BFFbPropsOptional
        if type == 'onscreen':
            flags = flags | GraphicsPipe.BFRequireWindow
        elif type == 'offscreen':
            flags = flags | GraphicsPipe.BFRefuseWindow

        if callbackWindowDict:
            flags = flags | GraphicsPipe.BFRequireCallbackWindow

        if host:
            assert host.isValid()
            win = self.graphicsEngine.makeOutput(pipe, name, 0, fbprops,
                                                 props, flags, host.getGsg(), host)
        elif gsg:
            win = self.graphicsEngine.makeOutput(pipe, name, 0, fbprops,
                                                 props, flags, gsg)
        else:
            win = self.graphicsEngine.makeOutput(pipe, name, 0, fbprops,
                                                 props, flags)

        if win is None:
            # Couldn't create a window!
            return None

        if unexposedDraw is not None and hasattr(win, 'setUnexposedDraw'):
            win.setUnexposedDraw(unexposedDraw)

        if callbackWindowDict:
            # If we asked for (and received) a CallbackGraphicsWindow,
            # we now have to assign the callbacks, before we start
            # trying to do anything with the window.
            for callbackName in ['Events', 'Properties', 'Render']:
                func = callbackWindowDict.get(callbackName, None)
                if not func:
                    continue

                setCallbackName = 'set%sCallback' % (callbackName)
                setCallback = getattr(win, setCallbackName)
                setCallback(PythonCallbackObject(func))

            # We also need to set up the mouse/keyboard objects.
            for inputName in callbackWindowDict.get('inputDevices', ['mouse']):
                win.createInputDevice(inputName)

        if hasattr(win, "requestProperties"):
            win.requestProperties(props)

        mainWindow = False
        if self.win is None:
            mainWindow = True
            self.win = win
            if hasattr(self, 'bufferViewer'):
                self.bufferViewer.win = win

        self.winList.append(win)

        # Set up a 3-d camera for the window by default.
        if keepCamera:
            self.makeCamera(win, scene = scene, aspectRatio = aspectRatio,
                            stereo = stereo, useCamera = self.cam)
        elif makeCamera:
            self.makeCamera(win, scene = scene, aspectRatio = aspectRatio,
                            stereo = stereo)

        messenger.send('open_window', [win, mainWindow])
        if mainWindow:
            messenger.send('open_main_window')

        return win

    def closeWindow(self, win, keepCamera = False, removeWindow = True):
        """
        Closes the indicated window and removes it from the list of
        windows.  If it is the main window, clears the main window
        pointer to None.
        """
        win.setActive(False)

        # First, remove all of the cameras associated with display
        # regions on the window.
        numRegions = win.getNumDisplayRegions()
        for i in range(numRegions):
            dr = win.getDisplayRegion(i)
            # [gjeon] remove drc in base.direct.drList
            if self.direct is not None:
                for drc in self.direct.drList:
                    if drc.cam == dr.getCamera():
                        self.direct.drList.displayRegionList.remove(drc)
                        break

            cam = NodePath(dr.getCamera())

            dr.setCamera(NodePath())

            if not cam.isEmpty() and \
               cam.node().getNumDisplayRegions() == 0 and \
               not keepCamera:
                # If the camera is used by no other DisplayRegions,
                # remove it.
                if self.camList.count(cam) != 0:
                    self.camList.remove(cam)

                # Don't throw away self.camera; we want to
                # preserve it for reopening the window.
                if cam == self.cam:
                    self.cam = None
                if cam == self.cam2d:
                    self.cam2d = None
                if cam == self.cam2dp:
                    self.cam2dp = None
                cam.removeNode()

        # [gjeon] remove winControl
        for winCtrl in self.winControls:
            if winCtrl.win == win:
                self.winControls.remove(winCtrl)
                break
        # Now we can actually close the window.
        if removeWindow:
            self.graphicsEngine.removeWindow(win)
        self.winList.remove(win)

        mainWindow = False
        if win == self.win:
            mainWindow = True
            self.win = None
            if self.frameRateMeter:
                self.frameRateMeter.clearWindow()
                self.frameRateMeter = None
            if self.sceneGraphAnalyzerMeter:
                self.sceneGraphAnalyzerMeter.clearWindow()
                self.sceneGraphAnalyzerMeter = None

        messenger.send('close_window', [win, mainWindow])
        if mainWindow:
            messenger.send('close_main_window')

        if not self.winList:
            # Give the window(s) a chance to actually close before we
            # continue.
            self.graphicsEngine.renderFrame()

    def openDefaultWindow(self, *args, **kw):
        """
        Creates the main window for the first time, without being too
        particular about the kind of graphics API that is chosen.
        The suggested window type from the load-display config variable is
        tried first; if that fails, the first window type that can be
        successfully opened at all is accepted.

        This is intended to be called only once, at application startup.
        It is normally called automatically unless window-type is configured
        to 'none'.

        :returns: True on success, False on failure.
        """

        startDirect = kw.get('startDirect', True)
        if 'startDirect' in kw:
            del kw['startDirect']

        self.openMainWindow(*args, **kw)

        if startDirect:
            self.__doStartDirect()

        return self.win is not None

    def openMainWindow(self, *args, **kw):
        """
        Creates the initial, main window for the application, and sets
        up the mouse and render2d structures appropriately for it.  If
        this method is called a second time, it will close the
        previous main window and open a new one, preserving the lens
        properties in base.camLens.

        :returns: True on success, or False on failure (in which case base.win
                  may be either None, or the previous, closed window).
        """
        keepCamera = kw.get('keepCamera', False)

        success = 1
        oldWin = self.win
        oldLens = self.camLens
        oldClearColorActive = None
        if self.win is not None:
            # Close the previous window.
            oldClearColorActive = self.win.getClearColorActive()
            oldClearColor = VBase4(self.win.getClearColor())
            oldClearDepthActive = self.win.getClearDepthActive()
            oldClearDepth = self.win.getClearDepth()
            oldClearStencilActive = self.win.getClearStencilActive()
            oldClearStencil = self.win.getClearStencil()
            self.closeWindow(self.win, keepCamera = keepCamera)

        # Open a new window.
        self.openWindow(*args, **kw)
        if self.win is None:
            self.win = oldWin
            self.winList.append(oldWin)
            success = 0

        if self.win is not None:
            if isinstance(self.win, GraphicsWindow):
                self.setupMouse(self.win)
            self.makeCamera2d(self.win)

            if self.wantRender2dp:
                self.makeCamera2dp(self.win)

            if oldLens is not None:
                # Restore the previous lens properties.
                self.camNode.setLens(oldLens)
                self.camLens = oldLens

            if oldClearColorActive is not None:
                # Restore the previous clear properties.
                self.win.setClearColorActive(oldClearColorActive)
                self.win.setClearColor(oldClearColor)
                self.win.setClearDepthActive(oldClearDepthActive)
                self.win.setClearDepth(oldClearDepth)
                self.win.setClearStencilActive(oldClearStencilActive)
                self.win.setClearStencil(oldClearStencil)

            flag = self.config.GetBool('show-frame-rate-meter', False)
            if self.appRunner is not None and self.appRunner.allowPythonDev:
                # In an allow_python_dev p3d application, we always
                # start up with the frame rate meter enabled, to
                # provide a visual reminder that this flag has been
                # set.
                flag = True
            self.setFrameRateMeter(flag)
            flag = self.config.GetBool('show-scene-graph-analyzer-meter', False)
            self.setSceneGraphAnalyzerMeter(flag)
        return success

    def setSleep(self, amount):
        """
        Sets up a task that calls python 'sleep' every frame.  This is a simple
        way to reduce the CPU usage (and frame rate) of a panda program.
        """
        if (self.clientSleep == amount):
            return
        self.clientSleep = amount
        if (amount == 0.0):
            self.taskMgr.remove('clientSleep')
        else:
            # Spawn it after igloop (at the end of each frame)
            self.taskMgr.remove('clientSleep')
            self.taskMgr.add(self.__sleepCycleTask, 'clientSleep', sort = 55)

    def __sleepCycleTask(self, task):
        Thread.sleep(self.clientSleep)
        #time.sleep(self.clientSleep)
        return Task.cont

    def setFrameRateMeter(self, flag):
        """
        Turns on or off (according to flag) a standard frame rate
        meter in the upper-right corner of the main window.
        """
        if flag:
            if not self.frameRateMeter:
                self.frameRateMeter = FrameRateMeter('frameRateMeter')
                self.frameRateMeter.setupWindow(self.win)
        else:
            if self.frameRateMeter:
                self.frameRateMeter.clearWindow()
                self.frameRateMeter = None

    def setSceneGraphAnalyzerMeter(self, flag):
        """
        Turns on or off (according to flag) a standard frame rate
        meter in the upper-right corner of the main window.
        """
        if flag:
            if not self.sceneGraphAnalyzerMeter:
                self.sceneGraphAnalyzerMeter = SceneGraphAnalyzerMeter('sceneGraphAnalyzerMeter', self.render.node())
                self.sceneGraphAnalyzerMeter.setupWindow(self.win)
        else:
            if self.sceneGraphAnalyzerMeter:
                self.sceneGraphAnalyzerMeter.clearWindow()
                self.sceneGraphAnalyzerMeter = None

    # [gjeon] now you can add more winControls after creating a showbase instance
    def setupWindowControls(self, winCtrl=None):
        if winCtrl is None:
            winCtrl = WindowControls(
                self.win, mouseWatcher=self.mouseWatcher,
                cam=self.camera, camNode = self.camNode, cam2d=self.camera2d,
                mouseKeyboard = self.dataRoot.find("**/*"))
        self.winControls.append(winCtrl)

    def setupRender(self):
        """
        Creates the render scene graph, the primary scene graph for
        rendering 3-d geometry.
        """
        #: This is the root of the 3-D scene graph.
        self.render = NodePath('render')
        self.render.setAttrib(RescaleNormalAttrib.makeDefault())

        self.render.setTwoSided(0)
        self.backfaceCullingEnabled = 1
        self.textureEnabled = 1
        self.wireframeEnabled = 0

    def setupRender2d(self):
        """
        Creates the render2d scene graph, the primary scene graph for
        2-d objects and gui elements that are superimposed over the
        3-d geometry in the window.
        """
        # We've already created render2d and aspect2d in ShowBaseGlobal,
        # for the benefit of creating DirectGui elements before ShowBase.
        from . import ShowBaseGlobal

        #: This is the root of the 2-D scene graph.
        self.render2d = ShowBaseGlobal.render2d

        # Set up some overrides to turn off certain properties which
        # we probably won't need for 2-d objects.

        # It's probably important to turn off the depth test, since
        # many 2-d objects will be drawn over each other without
        # regard to depth position.

        # We used to avoid clearing the depth buffer before drawing
        # render2d, but nowadays we clear it anyway, since we
        # occasionally want to put 3-d geometry under render2d, and
        # it's simplest (and seems to be easier on graphics drivers)
        # if the 2-d scene has been cleared first.

        self.render2d.setDepthTest(0)
        self.render2d.setDepthWrite(0)
        self.render2d.setMaterialOff(1)
        self.render2d.setTwoSided(1)

        #: The normal 2-d DisplayRegion has an aspect ratio that
        #: matches the window, but its coordinate system is square.
        #: This means anything we parent to render2d gets stretched.
        #: For things where that makes a difference, we set up
        #: aspect2d, which scales things back to the right aspect
        #: ratio along the X axis (Z is still from -1 to 1)
        self.aspect2d = ShowBaseGlobal.aspect2d

        aspectRatio = self.getAspectRatio()
        self.aspect2d.setScale(1.0 / aspectRatio, 1.0, 1.0)

        self.a2dBackground = self.aspect2d.attachNewNode("a2dBackground")

        #: The Z position of the top border of the aspect2d screen.
        self.a2dTop = 1.0
        #: The Z position of the bottom border of the aspect2d screen.
        self.a2dBottom = -1.0
        #: The X position of the left border of the aspect2d screen.
        self.a2dLeft = -aspectRatio
        #: The X position of the right border of the aspect2d screen.
        self.a2dRight = aspectRatio

        self.a2dTopCenter = self.aspect2d.attachNewNode("a2dTopCenter")
        self.a2dTopCenterNs = self.aspect2d.attachNewNode("a2dTopCenterNS")
        self.a2dBottomCenter = self.aspect2d.attachNewNode("a2dBottomCenter")
        self.a2dBottomCenterNs = self.aspect2d.attachNewNode("a2dBottomCenterNS")
        self.a2dLeftCenter = self.aspect2d.attachNewNode("a2dLeftCenter")
        self.a2dLeftCenterNs = self.aspect2d.attachNewNode("a2dLeftCenterNS")
        self.a2dRightCenter = self.aspect2d.attachNewNode("a2dRightCenter")
        self.a2dRightCenterNs = self.aspect2d.attachNewNode("a2dRightCenterNS")

        self.a2dTopLeft = self.aspect2d.attachNewNode("a2dTopLeft")
        self.a2dTopLeftNs = self.aspect2d.attachNewNode("a2dTopLeftNS")
        self.a2dTopRight = self.aspect2d.attachNewNode("a2dTopRight")
        self.a2dTopRightNs = self.aspect2d.attachNewNode("a2dTopRightNS")
        self.a2dBottomLeft = self.aspect2d.attachNewNode("a2dBottomLeft")
        self.a2dBottomLeftNs = self.aspect2d.attachNewNode("a2dBottomLeftNS")
        self.a2dBottomRight = self.aspect2d.attachNewNode("a2dBottomRight")
        self.a2dBottomRightNs = self.aspect2d.attachNewNode("a2dBottomRightNS")

        # Put the nodes in their places
        self.a2dTopCenter.setPos(0, 0, self.a2dTop)
        self.a2dTopCenterNs.setPos(0, 0, self.a2dTop)
        self.a2dBottomCenter.setPos(0, 0, self.a2dBottom)
        self.a2dBottomCenterNs.setPos(0, 0, self.a2dBottom)
        self.a2dLeftCenter.setPos(self.a2dLeft, 0, 0)
        self.a2dLeftCenterNs.setPos(self.a2dLeft, 0, 0)
        self.a2dRightCenter.setPos(self.a2dRight, 0, 0)
        self.a2dRightCenterNs.setPos(self.a2dRight, 0, 0)

        self.a2dTopLeft.setPos(self.a2dLeft, 0, self.a2dTop)
        self.a2dTopLeftNs.setPos(self.a2dLeft, 0, self.a2dTop)
        self.a2dTopRight.setPos(self.a2dRight, 0, self.a2dTop)
        self.a2dTopRightNs.setPos(self.a2dRight, 0, self.a2dTop)
        self.a2dBottomLeft.setPos(self.a2dLeft, 0, self.a2dBottom)
        self.a2dBottomLeftNs.setPos(self.a2dLeft, 0, self.a2dBottom)
        self.a2dBottomRight.setPos(self.a2dRight, 0, self.a2dBottom)
        self.a2dBottomRightNs.setPos(self.a2dRight, 0, self.a2dBottom)

        #: This special root, pixel2d, uses units in pixels that are relative
        #: to the window. The upperleft corner of the window is (0, 0),
        #: the lowerleft corner is (xsize, -ysize), in this coordinate system.
        self.pixel2d = self.render2d.attachNewNode(PGTop("pixel2d"))
        self.pixel2d.setPos(-1, 0, 1)
        xsize, ysize = self.getSize()
        if xsize > 0 and ysize > 0:
            self.pixel2d.setScale(2.0 / xsize, 1.0, 2.0 / ysize)

    def setupRender2dp(self):
        """
        Creates a render2d scene graph, the secondary scene graph for
        2-d objects and gui elements that are superimposed over the
        2-d and 3-d geometry in the window.
        """
        self.render2dp = NodePath('render2dp')

        # Set up some overrides to turn off certain properties which
        # we probably won't need for 2-d objects.

        # It's probably important to turn off the depth test, since
        # many 2-d objects will be drawn over each other without
        # regard to depth position.

        dt = DepthTestAttrib.make(DepthTestAttrib.MNone)
        dw = DepthWriteAttrib.make(DepthWriteAttrib.MOff)
        self.render2dp.setDepthTest(0)
        self.render2dp.setDepthWrite(0)

        self.render2dp.setMaterialOff(1)
        self.render2dp.setTwoSided(1)

        #: The normal 2-d DisplayRegion has an aspect ratio that
        #: matches the window, but its coordinate system is square.
        #: This means anything we parent to render2dp gets stretched.
        #: For things where that makes a difference, we set up
        #: aspect2dp, which scales things back to the right aspect
        #: ratio along the X axis (Z is still from -1 to 1)
        self.aspect2dp = self.render2dp.attachNewNode(PGTop("aspect2dp"))
        self.aspect2dp.node().setStartSort(16384)

        aspectRatio = self.getAspectRatio()
        self.aspect2dp.setScale(1.0 / aspectRatio, 1.0, 1.0)

        #: The Z position of the top border of the aspect2dp screen.
        self.a2dpTop = 1.0
        #: The Z position of the bottom border of the aspect2dp screen.
        self.a2dpBottom = -1.0
        #: The X position of the left border of the aspect2dp screen.
        self.a2dpLeft = -aspectRatio
        #: The X position of the right border of the aspect2dp screen.
        self.a2dpRight = aspectRatio

        self.a2dpTopCenter = self.aspect2dp.attachNewNode("a2dpTopCenter")
        self.a2dpBottomCenter = self.aspect2dp.attachNewNode("a2dpBottomCenter")
        self.a2dpLeftCenter = self.aspect2dp.attachNewNode("a2dpLeftCenter")
        self.a2dpRightCenter = self.aspect2dp.attachNewNode("a2dpRightCenter")

        self.a2dpTopLeft = self.aspect2dp.attachNewNode("a2dpTopLeft")
        self.a2dpTopRight = self.aspect2dp.attachNewNode("a2dpTopRight")
        self.a2dpBottomLeft = self.aspect2dp.attachNewNode("a2dpBottomLeft")
        self.a2dpBottomRight = self.aspect2dp.attachNewNode("a2dpBottomRight")

        # Put the nodes in their places
        self.a2dpTopCenter.setPos(0, 0, self.a2dpTop)
        self.a2dpBottomCenter.setPos(0, 0, self.a2dpBottom)
        self.a2dpLeftCenter.setPos(self.a2dpLeft, 0, 0)
        self.a2dpRightCenter.setPos(self.a2dpRight, 0, 0)

        self.a2dpTopLeft.setPos(self.a2dpLeft, 0, self.a2dpTop)
        self.a2dpTopRight.setPos(self.a2dpRight, 0, self.a2dpTop)
        self.a2dpBottomLeft.setPos(self.a2dpLeft, 0, self.a2dpBottom)
        self.a2dpBottomRight.setPos(self.a2dpRight, 0, self.a2dpBottom)

        #: This special root, pixel2dp, uses units in pixels that are relative
        #: to the window. The upperleft corner of the window is (0, 0),
        #: the lowerleft corner is (xsize, -ysize), in this coordinate system.
        self.pixel2dp = self.render2dp.attachNewNode(PGTop("pixel2dp"))
        self.pixel2dp.node().setStartSort(16384)
        self.pixel2dp.setPos(-1, 0, 1)
        xsize, ysize = self.getSize()
        if xsize > 0 and ysize > 0:
            self.pixel2dp.setScale(2.0 / xsize, 1.0, 2.0 / ysize)

    def setAspectRatio(self, aspectRatio):
        """ Sets the global aspect ratio of the main window.  Set it
        to None to restore automatic scaling. """
        self.__configAspectRatio = aspectRatio
        self.adjustWindowAspectRatio(self.getAspectRatio())

    def getAspectRatio(self, win = None):
        # Returns the actual aspect ratio of the indicated (or main
        # window), or the default aspect ratio if there is not yet a
        # main window.

        # If the config it set, we return that
        if self.__configAspectRatio:
            return self.__configAspectRatio

        aspectRatio = 1

        if win is None:
            win = self.win

        if win is not None and win.hasSize() and win.getSbsLeftYSize() != 0:
            aspectRatio = float(win.getSbsLeftXSize()) / float(win.getSbsLeftYSize())
        else:
            if win is None or not hasattr(win, "getRequestedProperties"):
                props = WindowProperties.getDefault()
            else:
                props = win.getRequestedProperties()
                if not props.hasSize():
                    props = WindowProperties.getDefault()

            if props.hasSize() and props.getYSize() != 0:
                aspectRatio = float(props.getXSize()) / float(props.getYSize())

        if aspectRatio == 0:
            return 1

        return aspectRatio

    def getSize(self, win = None):
        """
        Returns the actual size of the indicated (or main window), or the
        default size if there is not yet a main window.
        """

        if win is None:
            win = self.win

        if win is not None and win.hasSize():
            return win.getXSize(), win.getYSize()
        else:
            if win is None or not hasattr(win, "getRequestedProperties"):
                props = WindowProperties.getDefault()
            else:
                props = win.getRequestedProperties()
                if not props.hasSize():
                    props = WindowProperties.getDefault()

            return props.getXSize(), props.getYSize()

    def makeCamera(self, win, sort = 0, scene = None,
                   displayRegion = (0, 1, 0, 1), stereo = None,
                   aspectRatio = None, clearDepth = 0, clearColor = None,
                   lens = None, camName = 'cam', mask = None,
                   useCamera = None):
        """
        Makes a new 3-d camera associated with the indicated window,
        and creates a display region in the indicated subrectangle.

        If stereo is True, then a stereo camera is created, with a
        pair of DisplayRegions.  If stereo is False, then a standard
        camera is created.  If stereo is None or omitted, a stereo
        camera is created if the window says it can render in stereo.

        If useCamera is not None, it is a NodePath to be used as the
        camera to apply to the window, rather than creating a new
        camera.

        :rtype: panda3d.core.NodePath
        """
        # self.camera is the parent node of all cameras: a node that
        # we can move around to move all cameras as a group.
        if self.camera is None:
            # We make it a ModelNode with the PTLocal flag, so that
            # a wayward flatten operations won't attempt to mangle the
            # camera.
            self.camera = self.render.attachNewNode(ModelNode('camera'))
            self.camera.node().setPreserveTransform(ModelNode.PTLocal)
            builtins.camera = self.camera

            self.mouse2cam.node().setNode(self.camera.node())

        if useCamera:
            # Use the existing camera node.
            cam = useCamera
            camNode = useCamera.node()
            assert(isinstance(camNode, Camera))
            lens = camNode.getLens()
            cam.reparentTo(self.camera)

        else:
            # Make a new Camera node.
            camNode = Camera(camName)
            if lens is None:
                lens = PerspectiveLens()

                if aspectRatio is None:
                    aspectRatio = self.getAspectRatio(win)
                lens.setAspectRatio(aspectRatio)

            cam = self.camera.attachNewNode(camNode)

        if lens is not None:
            camNode.setLens(lens)

        if scene is not None:
            camNode.setScene(scene)

        if mask is not None:
            if (isinstance(mask, int)):
                mask = BitMask32(mask)
            camNode.setCameraMask(mask)

        if self.cam is None:
            self.cam = cam
            self.camNode = camNode
            self.camLens = lens

        self.camList.append(cam)

        # Now, make a DisplayRegion for the camera.
        if stereo is not None:
            if stereo:
                dr = win.makeStereoDisplayRegion(*displayRegion)
            else:
                dr = win.makeMonoDisplayRegion(*displayRegion)
        else:
            dr = win.makeDisplayRegion(*displayRegion)

        dr.setSort(sort)

        # By default, we do not clear 3-d display regions (the entire
        # window will be cleared, which is normally sufficient).  But
        # we will if clearDepth is specified.
        if clearDepth:
            dr.setClearDepthActive(1)

        if clearColor:
            dr.setClearColorActive(1)
            dr.setClearColor(clearColor)

        dr.setCamera(cam)

        return cam

    def makeCamera2d(self, win, sort = 10,
                     displayRegion = (0, 1, 0, 1), coords = (-1, 1, -1, 1),
                     lens = None, cameraName = None):
        """
        Makes a new camera2d associated with the indicated window, and
        assigns it to render the indicated subrectangle of render2d.

        :rtype: panda3d.core.NodePath
        """
        dr = win.makeMonoDisplayRegion(*displayRegion)
        dr.setSort(sort)

        # Enable clearing of the depth buffer on this new display
        # region (see the comment in setupRender2d, above).
        dr.setClearDepthActive(1)

        # Make any texture reloads on the gui come up immediately.
        dr.setIncompleteRender(False)

        left, right, bottom, top = coords

        # Now make a new Camera node.
        if (cameraName):
            cam2dNode = Camera('cam2d_' + cameraName)
        else:
            cam2dNode = Camera('cam2d')

        if lens is None:
            lens = OrthographicLens()
            lens.setFilmSize(right - left, top - bottom)
            lens.setFilmOffset((right + left) * 0.5, (top + bottom) * 0.5)
            lens.setNearFar(-1000, 1000)
        cam2dNode.setLens(lens)

        # self.camera2d is the analog of self.camera, although it's
        # not as clear how useful it is.
        if self.camera2d is None:
            self.camera2d = self.render2d.attachNewNode('camera2d')

        camera2d = self.camera2d.attachNewNode(cam2dNode)
        dr.setCamera(camera2d)

        if self.cam2d is None:
            self.cam2d = camera2d

        return camera2d

    def makeCamera2dp(self, win, sort = 20,
                      displayRegion = (0, 1, 0, 1), coords = (-1, 1, -1, 1),
                      lens = None, cameraName = None):
        """
        Makes a new camera2dp associated with the indicated window, and
        assigns it to render the indicated subrectangle of render2dp.

        :rtype: panda3d.core.NodePath
        """
        dr = win.makeMonoDisplayRegion(*displayRegion)
        dr.setSort(sort)

        # Unlike render2d, we don't clear the depth buffer for
        # render2dp.  Caveat emptor.

        if hasattr(dr, 'setIncompleteRender'):
            dr.setIncompleteRender(False)

        left, right, bottom, top = coords

        # Now make a new Camera node.
        if (cameraName):
            cam2dNode = Camera('cam2dp_' + cameraName)
        else:
            cam2dNode = Camera('cam2dp')

        if lens is None:
            lens = OrthographicLens()
            lens.setFilmSize(right - left, top - bottom)
            lens.setFilmOffset((right + left) * 0.5, (top + bottom) * 0.5)
            lens.setNearFar(-1000, 1000)
        cam2dNode.setLens(lens)

        # self.camera2d is the analog of self.camera, although it's
        # not as clear how useful it is.
        if self.camera2dp is None:
            self.camera2dp = self.render2dp.attachNewNode('camera2dp')

        camera2dp = self.camera2dp.attachNewNode(cam2dNode)
        dr.setCamera(camera2dp)

        if self.cam2dp is None:
            self.cam2dp = camera2dp

        return camera2dp

    def setupDataGraph(self):
        """
        Creates the data graph and populates it with the basic input
        devices.
        """
        self.dataRoot = NodePath('dataRoot')
        # Cache the node so we do not ask for it every frame
        self.dataRootNode = self.dataRoot.node()

        # Now we have the main trackball & drive interfaces.
        # useTrackball() and useDrive() switch these in and out; only
        # one is in use at a given time.
        self.trackball = NodePath(Trackball('trackball'))
        self.drive = NodePath(DriveInterface('drive'))
        self.mouse2cam = NodePath(Transform2SG('mouse2cam'))

    # [gjeon] now you can create multiple mouse watchers to support multiple windows
    def setupMouse(self, win, fMultiWin=False):
        """
        Creates the structures necessary to monitor the mouse input,
        using the indicated window.  If the mouse has already been set
        up for a different window, those structures are deleted first.

        :param fMultiWin: If True, then the previous mouse structures are not
                          deleted; instead, multiple windows are allowed to
                          monitor the mouse input.  However, in this case, the
                          trackball controls are not set up, and must be set up
                          by hand if desired.

        :returns: The ButtonThrower NodePath created for this window.
        """
        if not fMultiWin and self.buttonThrowers is not None:
            for bt in self.buttonThrowers:
                mw = bt.getParent()
                mk = mw.getParent()
                bt.removeNode()
                mw.removeNode()
                mk.removeNode()

        bts, pws = self.setupMouseCB(win)

        if fMultiWin:
            return bts[0]

        self.buttonThrowers = bts[:]
        self.pointerWatcherNodes = pws[:]

        self.mouseWatcher = self.buttonThrowers[0].getParent()
        self.mouseWatcherNode = self.mouseWatcher.node()

        if self.mouseInterface:
            self.mouseInterface.reparentTo(self.mouseWatcher)

        if self.recorder:
            # If we have a recorder, the mouseWatcher belongs under a
            # special MouseRecorder node, which may intercept the
            # mouse activity.
            mw = self.buttonThrowers[0].getParent()
            mouseRecorder = MouseRecorder('mouse')
            self.recorder.addRecorder('mouse', mouseRecorder)
            np = mw.getParent().attachNewNode(mouseRecorder)
            mw.reparentTo(np)


        mw = self.buttonThrowers[0].getParent()

        #: A special ButtonThrower to generate keyboard events and
        #: include the time from the OS.  This is separate only to
        #: support legacy code that did not expect a time parameter; it
        #: will eventually be folded into the normal ButtonThrower,
        #: above.
        self.timeButtonThrower = mw.attachNewNode(ButtonThrower('timeButtons'))
        self.timeButtonThrower.node().setPrefix('time-')
        self.timeButtonThrower.node().setTimeFlag(1)

        # Tell the gui system about our new mouse watcher.
        self.aspect2d.node().setMouseWatcher(mw.node())
        self.pixel2d.node().setMouseWatcher(mw.node())
        if self.wantRender2dp:
            self.aspect2dp.node().setMouseWatcher(mw.node())
            self.pixel2dp.node().setMouseWatcher(mw.node())

        mw.node().addRegion(PGMouseWatcherBackground())

        return self.buttonThrowers[0]

    # [gjeon] this function is seperated from setupMouse to allow multiple mouse watchers
    def setupMouseCB(self, win):
        # For each mouse/keyboard device, we create
        #  - MouseAndKeyboard
        #  - MouseWatcher
        #  - ButtonThrower
        # The ButtonThrowers are stored in a list, self.buttonThrowers.
        # Given a ButtonThrower, one can access the MouseWatcher and
        # MouseAndKeyboard using getParent.
        #
        # The MouseAndKeyboard generates mouse events and mouse
        # button/keyboard events; the MouseWatcher passes them through
        # unchanged when the mouse is not over a 2-d button, and passes
        # nothing through when the mouse *is* over a 2-d button.  Therefore,
        # objects that don't want to get events when the mouse is over a
        # button, like the driveInterface, should be parented to
        # MouseWatcher, while objects that want events in all cases, like the
        # chat interface, should be parented to the MouseAndKeyboard.

        buttonThrowers = []
        pointerWatcherNodes = []
        for i in range(win.getNumInputDevices()):
            name = win.getInputDeviceName(i)
            mk = self.dataRoot.attachNewNode(MouseAndKeyboard(win, i, name))
            mw = mk.attachNewNode(MouseWatcher("watcher%s" % (i)))

            if win.getSideBySideStereo():
                # If the window has side-by-side stereo enabled, then
                # we should constrain the MouseWatcher to the window's
                # DisplayRegion.  This will enable the MouseWatcher to
                # track the left and right halves of the screen
                # individually.
                mw.node().setDisplayRegion(win.getOverlayDisplayRegion())

            mb = mw.node().getModifierButtons()
            mb.addButton(KeyboardButton.shift())
            mb.addButton(KeyboardButton.control())
            mb.addButton(KeyboardButton.alt())
            mb.addButton(KeyboardButton.meta())
            mw.node().setModifierButtons(mb)
            bt = mw.attachNewNode(ButtonThrower("buttons%s" % (i)))
            if (i != 0):
                bt.node().setPrefix('mousedev%s-' % (i))
            mods = ModifierButtons()
            mods.addButton(KeyboardButton.shift())
            mods.addButton(KeyboardButton.control())
            mods.addButton(KeyboardButton.alt())
            mods.addButton(KeyboardButton.meta())
            bt.node().setModifierButtons(mods)
            buttonThrowers.append(bt)
            if (win.hasPointer(i)):
                pointerWatcherNodes.append(mw.node())

        return buttonThrowers, pointerWatcherNodes

    def enableSoftwareMousePointer(self):
        """
        Creates some geometry and parents it to render2d to show
        the currently-known mouse position.  Useful if the mouse
        pointer is invisible for some reason.
        """
        mouseViz = render2d.attachNewNode('mouseViz')
        lilsmiley = loader.loadModel('lilsmiley')
        lilsmiley.reparentTo(mouseViz)

        aspectRatio = self.getAspectRatio()
        # Scale the smiley face to 32x32 pixels.
        height = self.win.getSbsLeftYSize()
        lilsmiley.setScale(
            32.0 / height / aspectRatio,
            1.0, 32.0 / height)
        self.mouseWatcherNode.setGeometry(mouseViz.node())

    def getAlt(self):
        """
        Returns True if the alt key is currently held down.
        """
        return self.mouseWatcherNode.getModifierButtons().isDown(
            KeyboardButton.alt())

    def getShift(self):
        """
        Returns True if the shift key is currently held down.
        """
        return self.mouseWatcherNode.getModifierButtons().isDown(
            KeyboardButton.shift())

    def getControl(self):
        """
        Returns True if the control key is currently held down.
        """
        return self.mouseWatcherNode.getModifierButtons().isDown(
            KeyboardButton.control())

    def getMeta(self):
        """
        Returns True if the meta key is currently held down.
        """
        return self.mouseWatcherNode.getModifierButtons().isDown(
            KeyboardButton.meta())

    def attachInputDevice(self, device, prefix=None, watch=False):
        """
        This function attaches an input device to the data graph, which will
        cause the device to be polled and generate events.  If a prefix is
        given and not None, it is used to prefix events generated by this
        device, separated by a hyphen.

        The watch argument can be set to True (as of Panda3D 1.10.3) to set up
        the default MouseWatcher to receive inputs from this device, allowing
        it to be polled via mouseWatcherNode and control user interfaces.
        Setting this to True will also make it generate unprefixed events,
        regardless of the specified prefix.

        If you call this, you should consider calling detachInputDevice when
        you are done with the device or when it is disconnected.
        """

        # Protect against the same device being attached multiple times.
        assert device not in self.__inputDeviceNodes

        idn = self.dataRoot.attachNewNode(InputDeviceNode(device, device.name))

        # Setup the button thrower to generate events for the device.
        if prefix is not None or not watch:
            bt = idn.attachNewNode(ButtonThrower(device.name))
            if prefix is not None:
                bt.node().setPrefix(prefix + '-')
            self.deviceButtonThrowers.append(bt)

        assert self.notify.debug("Attached input device {0} with prefix {1}".format(device, prefix))
        self.__inputDeviceNodes[device] = idn

        if watch:
            idn.node().addChild(self.mouseWatcherNode)

    def detachInputDevice(self, device):
        """
        This should be called after attaching an input device using
        attachInputDevice and the device is disconnected or you no longer wish
        to keep polling this device for events.

        You do not strictly need to call this if you expect the device to be
        reconnected (but be careful that you don't reattach it).
        """

        if device not in self.__inputDeviceNodes:
            assert device in self.__inputDeviceNodes
            return

        assert self.notify.debug("Detached device {0}".format(device.name))

        # Remove the ButtonThrower from the deviceButtonThrowers list.
        idn = self.__inputDeviceNodes[device]
        for bt in self.deviceButtonThrowers:
            if idn.isAncestorOf(bt):
                self.deviceButtonThrowers.remove(bt)
                break

        idn.removeNode()
        del self.__inputDeviceNodes[device]

    def addAngularIntegrator(self):
        """
        Adds a :class:`~panda3d.physics.AngularEulerIntegrator` to the default
        physics manager.  By default, only a
        :class:`~panda3d.physics.LinearEulerIntegrator` is attached.
        """
        if not self.physicsMgrAngular:
            physics = importlib.import_module('panda3d.physics')
            self.physicsMgrAngular = 1
            integrator = physics.AngularEulerIntegrator()
            self.physicsMgr.attachAngularIntegrator(integrator)

    def enableParticles(self):
        """
        Enables the particle and physics managers, which are stored in
        `particleMgr` and `physicsMgr` members, respectively.  Also starts a
        task to periodically update these managers.

        By default, only a :class:`~panda3d.physics.LinearEulerIntegrator` is
        attached to the physics manager.  To attach an angular integrator,
        follow this up with a call to `addAngularIntegrator()`.
        """
        if not self.particleMgrEnabled:
            # Use importlib to prevent this import from being picked up
            # by modulefinder when packaging an application.

            if not self.particleMgr:
                PMG = importlib.import_module('direct.particles.ParticleManagerGlobal')
                self.particleMgr = PMG.particleMgr
                self.particleMgr.setFrameStepping(1)

            if not self.physicsMgr:
                PMG = importlib.import_module('direct.showbase.PhysicsManagerGlobal')
                physics = importlib.import_module('panda3d.physics')
                self.physicsMgr = PMG.physicsMgr
                integrator = physics.LinearEulerIntegrator()
                self.physicsMgr.attachLinearIntegrator(integrator)

            self.particleMgrEnabled = 1
            self.physicsMgrEnabled = 1
            self.taskMgr.remove('manager-update')
            self.taskMgr.add(self.updateManagers, 'manager-update')

    def disableParticles(self):
        """
        The opposite of `enableParticles()`.
        """
        if self.particleMgrEnabled:
            self.particleMgrEnabled = 0
            self.physicsMgrEnabled = 0
            self.taskMgr.remove('manager-update')

    def toggleParticles(self):
        """
        Calls `enableParticles()` or `disableParticles()` depending on the
        current state.
        """
        if self.particleMgrEnabled == 0:
            self.enableParticles()
        else:
            self.disableParticles()

    def isParticleMgrEnabled(self):
        """
        Returns True if `enableParticles()` has been called.
        """
        return self.particleMgrEnabled

    def isPhysicsMgrEnabled(self):
        """
        Returns True if `enableParticles()` has been called.
        """
        return self.physicsMgrEnabled

    def updateManagers(self, state):
        dt = globalClock.getDt()
        if (self.particleMgrEnabled == 1):
            self.particleMgr.doParticles(dt)
        if (self.physicsMgrEnabled == 1):
            self.physicsMgr.doPhysics(dt)
        return Task.cont

    def createStats(self, hostname=None, port=None):
        """
        If want-pstats is set in Config.prc, or the `wantStats` member is
        otherwise set to True, connects to the PStats server.
        This is normally called automatically from the ShowBase constructor.
        """
        # You can specify pstats-host in your Config.prc or use ~pstats/~aipstats
        # The default is localhost
        if not self.wantStats:
            return False

        if PStatClient.isConnected():
            PStatClient.disconnect()
        # these default values match the C++ default values
        if hostname is None:
            hostname = ''
        if port is None:
            port = -1
        PStatClient.connect(hostname, port)
        return PStatClient.isConnected()

    def addSfxManager(self, extraSfxManager):
        """
        Adds an additional SFX audio manager to `sfxManagerList`, the list of
        managers managed by ShowBase.
        """
        # keep a list of sfx manager objects to apply settings to,
        # since there may be others in addition to the one we create here
        self.sfxManagerList.append(extraSfxManager)
        newSfxManagerIsValid = (extraSfxManager!=None) and extraSfxManager.isValid()
        self.sfxManagerIsValidList.append(newSfxManagerIsValid)
        if newSfxManagerIsValid:
            extraSfxManager.setActive(self.sfxActive)

    def createBaseAudioManagers(self):
        """
        Creates the default SFX and music manager.  Called automatically from
        the ShowBase constructor.
        """
        self.sfxPlayer = SfxPlayer.SfxPlayer()
        sfxManager = AudioManager.createAudioManager()
        self.addSfxManager(sfxManager)

        self.musicManager = AudioManager.createAudioManager()
        self.musicManagerIsValid = self.musicManager is not None \
            and self.musicManager.isValid()
        if self.musicManagerIsValid:
            # ensure only 1 midi song is playing at a time:
            self.musicManager.setConcurrentSoundLimit(1)
            self.musicManager.setActive(self.musicActive)

    # enableMusic/enableSoundEffects are meant to be called in response
    # to a user request so sfxActive/musicActive represent how things
    # *should* be, regardless of App/OS/HW state
    def enableMusic(self, bEnableMusic):
        """
        Enables or disables the music manager.
        """
        # don't setActive(1) if no audiofocus
        if self.AppHasAudioFocus and self.musicManagerIsValid:
            self.musicManager.setActive(bEnableMusic)
        self.musicActive = bEnableMusic
        if bEnableMusic:
            # This is useful when we want to play different music
            # from what the manager has queued
            messenger.send("MusicEnabled")
            self.notify.debug("Enabling music")
        else:
            self.notify.debug("Disabling music")

    def SetAllSfxEnables(self, bEnabled):
        """Calls ``setActive(bEnabled)`` on all valid SFX managers."""
        for i in range(len(self.sfxManagerList)):
            if (self.sfxManagerIsValidList[i]):
                self.sfxManagerList[i].setActive(bEnabled)

    def enableSoundEffects(self, bEnableSoundEffects):
        """
        Enables or disables SFX managers.
        """
        # don't setActive(1) if no audiofocus
        if self.AppHasAudioFocus or (bEnableSoundEffects==0):
            self.SetAllSfxEnables(bEnableSoundEffects)
        self.sfxActive=bEnableSoundEffects
        if bEnableSoundEffects:
            self.notify.debug("Enabling sound effects")
        else:
            self.notify.debug("Disabling sound effects")

    # enable/disableAllAudio allow a programmable global override-off
    # for current audio settings.  they're meant to be called when app
    # loses audio focus (switched out), so we can turn off sound without
    # affecting internal sfxActive/musicActive sound settings, so things
    # come back ok when the app is switched back to

    def disableAllAudio(self):
        """
        Disables all SFX and music managers, meant to be called when the app
        loses audio focus.
        """
        self.AppHasAudioFocus = 0
        self.SetAllSfxEnables(0)
        if self.musicManagerIsValid:
            self.musicManager.setActive(0)
        self.notify.debug("Disabling audio")

    def enableAllAudio(self):
        """
        Reenables the SFX and music managers that were active at the time
        `disableAllAudio()` was called.  Meant to be called when the app regains
        audio focus.
        """
        self.AppHasAudioFocus = 1
        self.SetAllSfxEnables(self.sfxActive)
        if self.musicManagerIsValid:
            self.musicManager.setActive(self.musicActive)
        self.notify.debug("Enabling audio")

    # This function should only be in the loader but is here for
    # backwards compatibility. Please do not add code here, add
    # it to the loader.
    def loadSfx(self, name):
        """
        :deprecated: Use `.Loader.Loader.loadSfx()` instead.
        """
        assert self.notify.warning("base.loadSfx is deprecated, use base.loader.loadSfx instead.")
        return self.loader.loadSfx(name)

    # This function should only be in the loader but is here for
    # backwards compatibility. Please do not add code here, add
    # it to the loader.
    def loadMusic(self, name):
        """
        :deprecated: Use `.Loader.Loader.loadMusic()` instead.
        """
        assert self.notify.warning("base.loadMusic is deprecated, use base.loader.loadMusic instead.")
        return self.loader.loadMusic(name)

    def playSfx(
            self, sfx, looping = 0, interrupt = 1, volume = None,
            time = 0.0, node = None, listener = None, cutoff = None):
        # This goes through a special player for potential localization
        return self.sfxPlayer.playSfx(sfx, looping, interrupt, volume, time, node, listener, cutoff)

    def playMusic(self, music, looping = 0, interrupt = 1, volume = None, time = 0.0):
        if music:
            if volume is not None:
                music.setVolume(volume)

            # if interrupt was set to 0, start over even if it's
            # already playing
            if interrupt or (music.status() != AudioSound.PLAYING):
                music.setTime(time)
                music.setLoop(looping)
                music.play()

    def __resetPrevTransform(self, state):
        # Clear out the previous velocity deltas now, after we have
        # rendered (the previous frame).  We do this after the render,
        # so that we have a chance to draw a representation of spheres
        # along with their velocities.  At the beginning of the frame
        # really means after the command prompt, which allows the user
        # to interactively query these deltas meaningfully.

        PandaNode.resetAllPrevTransform()
        return Task.cont

    def __dataLoop(self, state):
        # Check if there were newly connected devices.
        self.devices.update()

        # traverse the data graph.  This reads all the control
        # inputs (from the mouse and keyboard, for instance) and also
        # directly acts upon them (for instance, to move the avatar).
        self.dgTrav.traverse(self.dataRootNode)
        return Task.cont

    def __ivalLoop(self, state):
        # Execute all intervals in the global ivalMgr.
        IntervalManager.ivalMgr.step()
        return Task.cont

    def initShadowTrav(self):
        if not self.shadowTrav:
            # set up the shadow collision traverser
            self.shadowTrav = CollisionTraverser("base.shadowTrav")
            self.shadowTrav.setRespectPrevTransform(False)

    def __shadowCollisionLoop(self, state):
        # run the collision traversal if we have a
        # CollisionTraverser set.
        if self.shadowTrav:
           self.shadowTrav.traverse(self.render)
        return Task.cont

    def __collisionLoop(self, state):
        # run the collision traversal if we have a
        # CollisionTraverser set.
        if self.cTrav:
            self.cTrav.traverse(self.render)
        if self.appTrav:
            self.appTrav.traverse(self.render)
        if self.shadowTrav:
           self.shadowTrav.traverse(self.render)
        messenger.send("collisionLoopFinished")
        return Task.cont

    def __audioLoop(self, state):
        if self.musicManager is not None:
            self.musicManager.update()
        for x in self.sfxManagerList:
            x.update()
        return Task.cont

    def __garbageCollectStates(self, state):
        """ This task is started only when we have
        garbage-collect-states set in the Config.prc file, in which
        case we're responsible for taking out Panda's garbage from
        time to time.  This is not to be confused with Python's
        garbage collection.  """

        TransformState.garbageCollect()
        RenderState.garbageCollect()
        return Task.cont

    def __igLoop(self, state):
        if __debug__:
            # We render the watch variables for the onScreenDebug as soon
            # as we reasonably can before the renderFrame().
            self.onScreenDebug.render()

        if self.recorder:
            self.recorder.recordFrame()

        # Finally, render the frame.
        self.graphicsEngine.renderFrame()
        if self.clusterSyncFlag:
            self.graphicsEngine.syncFrame()
        if self.multiClientSleep:
            time.sleep(0)

        if __debug__:
            # We clear the text buffer for the onScreenDebug as soon
            # as we reasonably can after the renderFrame().
            self.onScreenDebug.clear()

        if self.recorder:
            self.recorder.playFrame()

        if self.mainWinMinimized:
            # If the main window is minimized, slow down the app a bit
            # by sleeping here in igLoop so we don't use all available
            # CPU needlessly.

            # Note: this isn't quite right if multiple windows are
            # open.  We should base this on whether *all* windows are
            # minimized, not just the main window.  But it will do for
            # now until someone complains.
            time.sleep(0.1)

        # Lerp stuff needs this event, and it must be generated in
        # C++, not in Python.
        throw_new_frame()
        return Task.cont

    def __igLoopSync(self, state):
        if __debug__:
            # We render the watch variables for the onScreenDebug as soon
            # as we reasonably can before the renderFrame().
            self.onScreenDebug.render()

        if self.recorder:
            self.recorder.recordFrame()

        self.cluster.collectData()

        # Finally, render the frame.
        self.graphicsEngine.renderFrame()
        if self.clusterSyncFlag:
            self.graphicsEngine.syncFrame()
        if self.multiClientSleep:
            time.sleep(0)

        if __debug__:
            # We clear the text buffer for the onScreenDebug as soon
            # as we reasonably can after the renderFrame().
            self.onScreenDebug.clear()

        if self.recorder:
            self.recorder.playFrame()

        if self.mainWinMinimized:
            # If the main window is minimized, slow down the app a bit
            # by sleeping here in igLoop so we don't use all available
            # CPU needlessly.

            # Note: this isn't quite right if multiple windows are
            # open.  We should base this on whether *all* windows are
            # minimized, not just the main window.  But it will do for
            # now until someone complains.
            time.sleep(0.1)

        self.graphicsEngine.readyFlip()
        self.cluster.waitForFlipCommand()
        self.graphicsEngine.flipFrame()

        # Lerp stuff needs this event, and it must be generated in
        # C++, not in Python.
        throw_new_frame()
        return Task.cont

    def restart(self, clusterSync=False, cluster=None):
        self.shutdown()
        # __resetPrevTransform goes at the very beginning of the frame.
        self.taskMgr.add(
            self.__resetPrevTransform, 'resetPrevTransform', sort = -51)
        # give the dataLoop task a reasonably "early" sort,
        # so that it will get run before most tasks
        self.taskMgr.add(self.__dataLoop, 'dataLoop', sort = -50)
        self.__deadInputs = 0
        # spawn the ivalLoop with a later sort, so that it will
        # run after most tasks, but before igLoop.
        self.taskMgr.add(self.__ivalLoop, 'ivalLoop', sort = 20)
        # make the collisionLoop task run before igLoop,
        # but leave enough room for the app to insert tasks
        # between collisionLoop and igLoop
        self.taskMgr.add(self.__collisionLoop, 'collisionLoop', sort = 30)

        if ConfigVariableBool('garbage-collect-states').getValue():
            self.taskMgr.add(self.__garbageCollectStates, 'garbageCollectStates', sort = 46)
        # give the igLoop task a reasonably "late" sort,
        # so that it will get run after most tasks
        self.cluster = cluster
        if not clusterSync or cluster is None:
            self.taskMgr.add(self.__igLoop, 'igLoop', sort = 50)
        else:
            self.taskMgr.add(self.__igLoopSync, 'igLoop', sort = 50)
        # the audioLoop updates the positions of 3D sounds.
        # as such, it needs to run after the cull traversal in the igLoop.
        self.taskMgr.add(self.__audioLoop, 'audioLoop', sort = 60)
        self.eventMgr.restart()

    def shutdown(self):
        self.taskMgr.remove('audioLoop')
        self.taskMgr.remove('igLoop')
        self.taskMgr.remove('shadowCollisionLoop')
        self.taskMgr.remove('collisionLoop')
        self.taskMgr.remove('dataLoop')
        self.taskMgr.remove('resetPrevTransform')
        self.taskMgr.remove('ivalLoop')
        self.taskMgr.remove('garbageCollectStates')
        self.eventMgr.shutdown()

    def getBackgroundColor(self, win = None):
        """
        Returns the current window background color.  This assumes
        the window is set up to clear the color each frame (this is
        the normal setting).

        :rtype: panda3d.core.VBase4
        """
        if win is None:
            win = self.win

        return VBase4(win.getClearColor())

    def setBackgroundColor(self, r = None, g = None, b = None, a = 0.0, win = None):
        """
        Sets the window background color to the indicated value.
        This assumes the window is set up to clear the color each
        frame (this is the normal setting).

        The color may be either a VBase3 or a VBase4, or a 3-component
        tuple, or the individual r, g, b parameters.
        """
        if g is not None:
            color = VBase4(r, g, b, a)
        else:
            arg = r
            if isinstance(arg, VBase4):
                color = arg
            else:
                color = VBase4(arg[0], arg[1], arg[2], a)

        if win is None:
            win = self.win

        if win:
            win.setClearColor(color)

    def toggleBackface(self):
        """
        Toggles between `backfaceCullingOn()` and `backfaceCullingOff()`.
        """
        if self.backfaceCullingEnabled:
            self.backfaceCullingOff()
        else:
            self.backfaceCullingOn()

    def backfaceCullingOn(self):
        """
        Disables two-sided rendering on the entire 3D scene graph.
        """
        if not self.backfaceCullingEnabled:
            self.render.setTwoSided(0)
        self.backfaceCullingEnabled = 1

    def backfaceCullingOff(self):
        """
        Enables two-sided rendering on the entire 3D scene graph.
        """
        if self.backfaceCullingEnabled:
            self.render.setTwoSided(1)
        self.backfaceCullingEnabled = 0

    def toggleTexture(self):
        """
        Toggles between `textureOn()` and `textureOff()`.
        """
        if self.textureEnabled:
            self.textureOff()
        else:
            self.textureOn()

    def textureOn(self):
        """
        Undoes the effects of a previous call to `textureOff()`.
        """
        self.render.clearTexture()
        self.textureEnabled = 1

    def textureOff(self):
        """
        Disables texturing on the entire 3D scene graph.
        """
        self.render.setTextureOff(100)
        self.textureEnabled = 0

    def toggleWireframe(self):
        """
        Toggles between `wireframeOn()` and `wireframeOff()`.
        """
        if self.wireframeEnabled:
            self.wireframeOff()
        else:
            self.wireframeOn()

    def wireframeOn(self):
        """
        Enables wireframe rendering on the entire 3D scene graph.
        """
        self.render.setRenderModeWireframe(100)
        self.render.setTwoSided(1)
        self.wireframeEnabled = 1

    def wireframeOff(self):
        """
        Undoes the effects of a previous call to `wireframeOn()`.
        """
        self.render.clearRenderMode()
        render.setTwoSided(not self.backfaceCullingEnabled)
        self.wireframeEnabled = 0

    def disableMouse(self):
        """
        Temporarily disable the mouse control of the camera, either
        via the drive interface or the trackball, whichever is
        currently in use.
        """
        # We don't reparent the drive interface or the trackball;
        # whichever one was there before will remain in the data graph
        # and active.  This way they won't lose button events while
        # the mouse is disabled.  However, we do move the mouse2cam
        # object out of there, so we won't be updating the camera any
        # more.
        if self.mouse2cam:
            self.mouse2cam.detachNode()

    def enableMouse(self):
        """
        Reverse the effect of a previous call to `disableMouse()`.
        `useDrive()` also implicitly enables the mouse.
        """
        if self.mouse2cam:
            self.mouse2cam.reparentTo(self.mouseInterface)

    def silenceInput(self):
        """
        This is a heavy-handed way of temporarily turning off
        all inputs.  Bring them back with `reviveInput()`.
        """
        if not self.__deadInputs:
            self.__deadInputs = taskMgr.remove('dataLoop')

    def reviveInput(self):
        """
        Restores inputs after a previous call to `silenceInput()`.
        """
        if self.__deadInputs:
            self.eventMgr.doEvents()
            self.dgTrav.traverse(self.dataRootNode)
            self.eventMgr.eventQueue.clear()
            self.taskMgr.add(self.__dataLoop, 'dataLoop', sort = -50)
            self.__deadInputs = 0

    def setMouseOnNode(self, newNode):
        if self.mouse2cam:
            self.mouse2cam.node().setNode(newNode)

    def changeMouseInterface(self, changeTo):
        """
        Change the mouse interface used to control the camera.
        """
        # Get rid of the prior interface:
        self.mouseInterface.detachNode()
        # Update the mouseInterface to point to the drive
        self.mouseInterface = changeTo
        self.mouseInterfaceNode = self.mouseInterface.node()
        # Hookup the drive to the camera.
        if self.mouseWatcher:
            self.mouseInterface.reparentTo(self.mouseWatcher)
        if self.mouse2cam:
            self.mouse2cam.reparentTo(self.mouseInterface)

    def useDrive(self):
        """
        Changes the mouse interface used for camera control to drive mode.
        """
        if self.drive:
            self.changeMouseInterface(self.drive)
            # Set the height to a good eyeheight
            self.mouseInterfaceNode.reset()
            self.mouseInterfaceNode.setZ(4.0)

    def useTrackball(self):
        """
        Changes the mouse interface used for camera control to trackball mode.
        """
        if self.trackball:
            self.changeMouseInterface(self.trackball)

    def toggleTexMem(self):
        """
        Toggles a handy texture memory watcher utility.
        See :mod:`direct.showutil.TexMemWatcher` for more information.
        """

        if self.texmem and not self.texmem.cleanedUp:
            self.texmem.cleanup()
            self.texmem = None
            return

        # Use importlib to prevent this import from being picked up
        # by modulefinder when packaging an application.
        TMW = importlib.import_module('direct.showutil.TexMemWatcher')
        self.texmem = TMW.TexMemWatcher()

    def toggleShowVertices(self):
        """ Toggles a mode that visualizes vertex density per screen
        area. """

        if self.showVertices:
            # Clean up the old mode.
            self.showVertices.node().setActive(0)
            dr = self.showVertices.node().getDisplayRegion(0)
            self.win.removeDisplayRegion(dr)
            self.showVertices.removeNode()
            self.showVertices = None
            return

        dr = self.win.makeDisplayRegion()
        dr.setSort(1000)
        cam = Camera('showVertices')
        cam.setLens(self.camLens)

        # Set up a funny state to render only vertices.
        override = 100000
        t = NodePath('t')
        t.setColor(1, 0, 1, 0.02, override)
        t.setColorScale(1, 1, 1, 1, override)
        t.setAttrib(ColorBlendAttrib.make(ColorBlendAttrib.MAdd, ColorBlendAttrib.OIncomingAlpha, ColorBlendAttrib.OOneMinusIncomingAlpha), override)
        t.setAttrib(RenderModeAttrib.make(RenderModeAttrib.MPoint, 10), override)
        t.setTwoSided(True, override)
        t.setBin('fixed', 0, override)
        t.setDepthTest(False, override)
        t.setDepthWrite(False, override)
        t.setLightOff(override)
        t.setShaderOff(override)
        t.setFogOff(override)
        t.setAttrib(AntialiasAttrib.make(AntialiasAttrib.MNone), override)
        t.setAttrib(RescaleNormalAttrib.make(RescaleNormalAttrib.MNone), override)
        t.setTextureOff(override)

        # Make the spots round, so there's less static in the display.
        # This forces software point generation on many drivers, so
        # it's not on by default.
        if self.config.GetBool('round-show-vertices', False):
            spot = PNMImage(256, 256, 1)
            spot.renderSpot((1, 1, 1, 1), (0, 0, 0, 0), 0.8, 1)
            tex = Texture('spot')
            tex.load(spot)
            tex.setFormat(tex.FAlpha)
            t.setTexture(tex, override)
            t.setAttrib(TexGenAttrib.make(TextureStage.getDefault(), TexGenAttrib.MPointSprite), override)

        cam.setInitialState(t.getState())
        cam.setCameraMask(~PandaNode.getOverallBit())

        self.showVertices = self.cam.attachNewNode(cam)
        dr.setCamera(self.showVertices)

    def oobe(self, cam = None):
        """
        Enable a special "out-of-body experience" mouse-interface
        mode.  This can be used when a "god" camera is needed; it
        moves the camera node out from under its normal node and sets
        the world up in trackball state.  Button events are still sent
        to the normal mouse action node (e.g. the DriveInterface), and
        mouse events, if needed, may be sent to the normal node by
        holding down the Control key.

        This is different than `useTrackball()`, which simply changes
        the existing mouse action to a trackball interface.  In fact,
        OOBE mode doesn't care whether `useDrive()` or `useTrackball()` is
        in effect; it just temporarily layers a new trackball
        interface on top of whatever the basic interface is.  You can
        even switch between `useDrive()` and `useTrackball()` while OOBE
        mode is in effect.

        This is a toggle; the second time this function is called, it
        disables the mode.
        """
        if cam is None:
            cam = self.cam

        # If oobeMode was never set, set it to false and create the
        # structures we need to implement OOBE.
        if not hasattr(self, 'oobeMode'):
            self.oobeMode = 0

            self.oobeCamera = self.hidden.attachNewNode('oobeCamera')
            self.oobeCameraTrackball = self.oobeCamera.attachNewNode('oobeCameraTrackball')
            self.oobeLens = PerspectiveLens()
            self.oobeLens.setAspectRatio(self.getAspectRatio())
            self.oobeLens.setNearFar(0.1, 10000.0)
            self.oobeLens.setMinFov(40)

            self.oobeTrackball = NodePath(Trackball('oobeTrackball'))
            self.oobe2cam = self.oobeTrackball.attachNewNode(Transform2SG('oobe2cam'))
            self.oobe2cam.node().setNode(self.oobeCameraTrackball.node())

            self.oobeVis = loader.loadModel('models/misc/camera', okMissing = True)
            if not self.oobeVis:
                # Sometimes we have default-model-extension set to
                # egg, but the file might be a bam file.
                self.oobeVis = loader.loadModel('models/misc/camera.bam', okMissing = True)
            if not self.oobeVis:
                self.oobeVis = NodePath('oobeVis')
            self.oobeVis.node().setFinal(1)
            self.oobeVis.setLightOff(1)
            self.oobeCullFrustum = None

            self.accept('oobe-down', self.__oobeButton, extraArgs = [''])
            self.accept('oobe-repeat', self.__oobeButton, extraArgs = ['-repeat'])
            self.accept('oobe-up', self.__oobeButton, extraArgs = ['-up'])

        if self.oobeMode:
            # Disable OOBE mode.
            if self.oobeCullFrustum is not None:
                # First, disable OOBE cull mode.
                self.oobeCull(cam = cam)

            if self.oobeVis:
                self.oobeVis.reparentTo(self.hidden)

            # Restore the mouse interface node, and remove the oobe
            # trackball from the data path.
            self.mouseInterfaceNode.clearButton(KeyboardButton.shift())
            self.oobeTrackball.detachNode()

            bt = self.buttonThrowers[0].node()
            bt.setSpecificFlag(1)
            bt.setButtonDownEvent('')
            bt.setButtonRepeatEvent('')
            bt.setButtonUpEvent('')

            cam.reparentTo(self.camera)
            #if cam == self.cam:
            #    self.camNode.setLens(self.camLens)
            self.oobeCamera.reparentTo(self.hidden)
            self.oobeMode = 0
            bboard.post('oobeEnabled', False)
        else:
            bboard.post('oobeEnabled', True)
            try:
                cameraParent = localAvatar
            except:
                # Make oobeCamera be a sibling of wherever camera is now.
                cameraParent = self.camera.getParent()
            self.oobeCamera.reparentTo(cameraParent)
            self.oobeCamera.clearMat()

            # Make the regular MouseInterface node respond only when
            # the shift button is pressed.  And the oobe node will
            # respond only when shift is *not* pressed.

            self.mouseInterfaceNode.requireButton(KeyboardButton.shift(), True)
            self.oobeTrackball.node().requireButton(KeyboardButton.shift(), False)
            self.oobeTrackball.reparentTo(self.mouseWatcher)

            # Set our initial OOB position to be just behind the camera.
            mat = Mat4.translateMat(0, -10, 3) * self.camera.getMat(cameraParent)
            mat.invertInPlace()
            self.oobeTrackball.node().setMat(mat)

            cam.reparentTo(self.oobeCameraTrackball)

            # Temporarily disable button events by routing them
            # through the oobe filters.
            bt = self.buttonThrowers[0].node()
            bt.setSpecificFlag(0)
            bt.setButtonDownEvent('oobe-down')
            bt.setButtonRepeatEvent('oobe-repeat')
            bt.setButtonUpEvent('oobe-up')

            # Don't change the camera lens--keep it with the original lens.
            #if cam == self.cam:
            #    self.camNode.setLens(self.oobeLens)

            if self.oobeVis:
                self.oobeVis.reparentTo(self.camera)
            self.oobeMode = 1

    def __oobeButton(self, suffix, button):
        if button.startswith('mouse'):
            # Eat mouse buttons.
            return

        # Transmit other buttons.
        messenger.send(button + suffix)

    def oobeCull(self, cam = None):
        """
        While in OOBE mode (see above), cull the viewing frustum as if
        it were still attached to our original camera.  This allows us
        to visualize the effectiveness of our bounding volumes.
        """
        if cam is None:
            cam = self.cam

        # First, make sure OOBE mode is enabled.
        if not getattr(self, 'oobeMode', False):
            self.oobe(cam = cam)

        if self.oobeCullFrustum is None:
            # Enable OOBE culling.
            pnode = LensNode('oobeCull')
            pnode.setLens(self.camLens)
            pnode.showFrustum()
            self.oobeCullFrustum = self.camera.attachNewNode(pnode)

            # Tell the camera to cull from here instead of its own
            # origin.
            for c in self.camList:
                c.node().setCullCenter(self.oobeCullFrustum)
            if cam.node().isOfType(Camera):
                cam.node().setCullCenter(self.oobeCullFrustum)
            for c in cam.findAllMatches('**/+Camera'):
                c.node().setCullCenter(self.oobeCullFrustum)
        else:
            # Disable OOBE culling.

            for c in self.camList:
                c.node().setCullCenter(NodePath())
            if cam.node().isOfType(Camera):
                cam.node().setCullCenter(self.oobeCullFrustum)
            for c in cam.findAllMatches('**/+Camera'):
                c.node().setCullCenter(NodePath())
            self.oobeCullFrustum.removeNode()
            self.oobeCullFrustum = None

    def showCameraFrustum(self):
        # Create a visible representation of the frustum.
        self.removeCameraFrustum()
        geom = self.camLens.makeGeometry()
        if geom is not None:
            gn = GeomNode('frustum')
            gn.addGeom(geom)
            self.camFrustumVis = self.camera.attachNewNode(gn)

    def removeCameraFrustum(self):
        if self.camFrustumVis:
            self.camFrustumVis.removeNode()

    def screenshot(self, namePrefix = 'screenshot',
                   defaultFilename = 1, source = None,
                   imageComment=""):
        """ Captures a screenshot from the main window or from the
        specified window or Texture and writes it to a filename in the
        current directory (or to a specified directory).

        If defaultFilename is True, the filename is synthesized by
        appending namePrefix to a default filename suffix (including
        the filename extension) specified in the Config variable
        screenshot-filename.  Otherwise, if defaultFilename is False,
        the entire namePrefix is taken to be the filename to write,
        and this string should include a suitable filename extension
        that will be used to determine the type of image file to
        write.

        Normally, the source is a GraphicsWindow, GraphicsBuffer or
        DisplayRegion.  If a Texture is supplied instead, it must have
        a ram image (that is, if it was generated by
        makeTextureBuffer() or makeCubeMap(), the parameter toRam
        should have been set true).  If it is a cube map texture as
        generated by makeCubeMap(), namePrefix should contain the hash
        mark ('#') character.

        :returns: The filename if successful, or None if there is a problem.
        """

        if source is None:
            source = self.win

        if defaultFilename:
            filename = GraphicsOutput.makeScreenshotFilename(namePrefix)
        else:
            filename = Filename(namePrefix)

        if isinstance(source, Texture):
            if source.getZSize() > 1:
                saved = source.write(filename, 0, 0, 1, 0)
            else:
                saved = source.write(filename)
        else:
            saved = source.saveScreenshot(filename, imageComment)

        if saved:
            # Announce to anybody that a screenshot has been taken
            messenger.send('screenshot', [filename])
            return filename

        return None

    def saveCubeMap(self, namePrefix = 'cube_map_#.png',
                    defaultFilename = 0, source = None,
                    camera = None, size = 128,
                    cameraMask = PandaNode.getAllCameraMask(),
                    sourceLens = None):

        """
        Similar to :meth:`screenshot()`, this sets up a temporary cube
        map Texture which it uses to take a series of six snapshots of
        the current scene, one in each of the six cube map directions.
        This requires rendering a new frame.

        Unlike `screenshot()`, source may only be a GraphicsWindow,
        GraphicsBuffer, or DisplayRegion; it may not be a Texture.

        camera should be the node to which the cubemap cameras will be
        parented.  The default is the camera associated with source,
        if source is a DisplayRegion, or base.camera otherwise.

        :returns: The filename if successful, or None if there is a problem.
        """

        if source is None:
            source = self.win

        if camera is None:
            if hasattr(source, "getCamera"):
                camera = source.getCamera()
            if camera is None:
                camera = self.camera

        if sourceLens is None:
            sourceLens = self.camLens

        if hasattr(source, "getWindow"):
            source = source.getWindow()

        rig = NodePath(namePrefix)
        buffer = source.makeCubeMap(namePrefix, size, rig, cameraMask, 1)
        if buffer is None:
            raise Exception("Could not make cube map.")

        # Set the near and far planes from the default lens.
        lens = rig.find('**/+Camera').node().getLens()

        lens.setNearFar(sourceLens.getNear(), sourceLens.getFar())

        # Now render a frame to fill up the texture.
        rig.reparentTo(camera)
        self.graphicsEngine.openWindows()
        self.graphicsEngine.renderFrame()
        self.graphicsEngine.renderFrame()
        self.graphicsEngine.syncFrame()

        tex = buffer.getTexture()
        saved = self.screenshot(namePrefix = namePrefix,
                                defaultFilename = defaultFilename,
                                source = tex)

        self.graphicsEngine.removeWindow(buffer)
        rig.removeNode()

        return saved

    def saveSphereMap(self, namePrefix = 'spheremap.png',
                      defaultFilename = 0, source = None,
                      camera = None, size = 256,
                      cameraMask = PandaNode.getAllCameraMask(),
                      numVertices = 1000, sourceLens = None):
        """
        This works much like :meth:`saveCubeMap()`, and uses the
        graphics API's hardware cube-mapping ability to get a 360-degree
        view of the world.  But then it converts the six cube map faces
        into a single fisheye texture, suitable for applying as a static
        environment map (sphere map).

        For eye-relative static environment maps, sphere maps are often
        preferable to cube maps because they require only a single
        texture and because they are supported on a broader range of
        hardware.

        :returns: The filename if successful, or None if there is a problem.
        """
        if source is None:
            source = self.win

        if camera is None:
            if hasattr(source, "getCamera"):
                camera = source.getCamera()
            if camera is None:
                camera = self.camera

        if sourceLens is None:
            sourceLens = self.camLens

        if hasattr(source, "getWindow"):
            source = source.getWindow()

        # First, make an offscreen buffer to convert the cube map to a
        # sphere map.  We make it first so we can guarantee the
        # rendering order for the cube map.
        toSphere = source.makeTextureBuffer(namePrefix, size, size,
                                            Texture(), 1)

        # Now make the cube map buffer.
        rig = NodePath(namePrefix)
        buffer = toSphere.makeCubeMap(namePrefix, size, rig, cameraMask, 0)
        if buffer is None:
            self.graphicsEngine.removeWindow(toSphere)
            raise Exception("Could not make cube map.")

        # Set the near and far planes from the default lens.
        lens = rig.find('**/+Camera').node().getLens()
        lens.setNearFar(sourceLens.getNear(), sourceLens.getFar())

        # Set up the scene to convert the cube map.  It's just a
        # simple scene, with only the FisheyeMaker object in it.
        dr = toSphere.makeMonoDisplayRegion()
        camNode = Camera('camNode')
        lens = OrthographicLens()
        lens.setFilmSize(2, 2)
        lens.setNearFar(-1000, 1000)
        camNode.setLens(lens)
        root = NodePath('buffer')
        cam = root.attachNewNode(camNode)
        dr.setCamera(cam)

        fm = FisheyeMaker('card')
        fm.setNumVertices(numVertices)
        fm.setSquareInscribed(1, 1.1)
        fm.setReflection(1)
        card = root.attachNewNode(fm.generate())
        card.setTexture(buffer.getTexture())

        # Now render a frame.  This will render out the cube map and
        # then apply it to the the card in the toSphere buffer.
        rig.reparentTo(camera)
        self.graphicsEngine.openWindows()
        self.graphicsEngine.renderFrame()

        # One more frame for luck.
        self.graphicsEngine.renderFrame()
        self.graphicsEngine.syncFrame()

        saved = self.screenshot(namePrefix = namePrefix,
                                defaultFilename = defaultFilename,
                                source = toSphere.getTexture())

        self.graphicsEngine.removeWindow(buffer)
        self.graphicsEngine.removeWindow(toSphere)
        rig.removeNode()

        return saved

    def movie(self, namePrefix = 'movie', duration = 1.0, fps = 30,
              format = 'png', sd = 4, source = None):
        """
        Spawn a task to capture a movie using the screenshot function.

        Args:
            namePrefix (str): used to form output file names (can
                include path information (e.g. '/i/beta/frames/myMovie')
            duration (float): the length of the movie in seconds
            fps (float): the frame rate of the resulting movie
            format (str): specifies output file format (e.g. png, bmp)
            sd (int): specifies number of significant digits for frame
                count in the output file name (e.g. if sd = 4, the name
                will be something like movie_0001.png)
            source: the Window, Buffer, DisplayRegion, or Texture from
                which to save the resulting images.  The default is the
                main window.

        Returns:
            A `~direct.task.Task` that can be awaited.
        """
        globalClock.setMode(ClockObject.MNonRealTime)
        globalClock.setDt(1.0/float(fps))
        t = self.taskMgr.add(self._movieTask, namePrefix + '_task')
        t.frameIndex = 0  # Frame 0 is not captured.
        t.numFrames = int(duration * fps)
        t.source = source
        t.outputString = namePrefix + '_%0' + repr(sd) + 'd.' + format
        t.setUponDeath(lambda state: globalClock.setMode(ClockObject.MNormal))
        return t

    def _movieTask(self, state):
        if state.frameIndex != 0:
            frameName = state.outputString % state.frameIndex
            self.notify.info("Capturing frame: " + frameName)
            self.screenshot(namePrefix = frameName, defaultFilename = 0,
                            source = state.source)

        state.frameIndex += 1
        if state.frameIndex > state.numFrames:
            return Task.done
        else:
            return Task.cont

    def windowEvent(self, win):
        if win != self.win:
            # This event isn't about our window.
            return

        properties = win.getProperties()
        if properties != self.__prevWindowProperties:
            self.__prevWindowProperties = properties

            self.notify.debug("Got window event: %s" % (repr(properties)))
            if not properties.getOpen():
                # If the user closes the main window, we should exit.
                self.notify.info("User closed main window.")
                if __debug__:
                    if self.__autoGarbageLogging:
                        GarbageReport.b_checkForGarbageLeaks()
                self.userExit()

            if properties.getForeground() and not self.mainWinForeground:
                self.mainWinForeground = 1
            elif not properties.getForeground() and self.mainWinForeground:
                self.mainWinForeground = 0
                if __debug__:
                    if self.__autoGarbageLogging:
                        GarbageReport.b_checkForGarbageLeaks()

            if properties.getMinimized() and not self.mainWinMinimized:
                # If the main window is minimized, throw an event to
                # stop the music.
                self.mainWinMinimized = 1
                messenger.send('PandaPaused')
            elif not properties.getMinimized() and self.mainWinMinimized:
                # If the main window is restored, throw an event to
                # restart the music.
                self.mainWinMinimized = 0
                messenger.send('PandaRestarted')

            # If we have not forced the aspect ratio, let's see if it has
            # changed and update the camera lenses and aspect2d parameters
            self.adjustWindowAspectRatio(self.getAspectRatio())

            if win.hasSize() and win.getSbsLeftYSize() != 0:
                self.pixel2d.setScale(2.0 / win.getSbsLeftXSize(), 1.0, 2.0 / win.getSbsLeftYSize())
                if self.wantRender2dp:
                    self.pixel2dp.setScale(2.0 / win.getSbsLeftXSize(), 1.0, 2.0 / win.getSbsLeftYSize())
            else:
                xsize, ysize = self.getSize()
                if xsize > 0 and ysize > 0:
                    self.pixel2d.setScale(2.0 / xsize, 1.0, 2.0 / ysize)
                    if self.wantRender2dp:
                        self.pixel2dp.setScale(2.0 / xsize, 1.0, 2.0 / ysize)

    def adjustWindowAspectRatio(self, aspectRatio):
        """ This function is normally called internally by
        `windowEvent()`, but it may also be called to explicitly adjust
        the aspect ratio of the render/render2d DisplayRegion, by a
        class that has redefined these. """

        if self.__configAspectRatio:
            aspectRatio = self.__configAspectRatio

        if aspectRatio != self.__oldAspectRatio:
            self.__oldAspectRatio = aspectRatio
            # Fix up some anything that depends on the aspectRatio
            if self.camLens:
                self.camLens.setAspectRatio(aspectRatio)
            if aspectRatio < 1:
                # If the window is TALL, lets expand the top and bottom
                self.aspect2d.setScale(1.0, aspectRatio, aspectRatio)
                self.a2dTop = 1.0 / aspectRatio
                self.a2dBottom = - 1.0 / aspectRatio
                self.a2dLeft = -1
                self.a2dRight = 1.0
                # Don't forget 2dp
                if self.wantRender2dp:
                    self.aspect2dp.setScale(1.0, aspectRatio, aspectRatio)
                    self.a2dpTop = 1.0 / aspectRatio
                    self.a2dpBottom = - 1.0 / aspectRatio
                    self.a2dpLeft = -1
                    self.a2dpRight = 1.0

            else:
                # If the window is WIDE, lets expand the left and right
                self.aspect2d.setScale(1.0 / aspectRatio, 1.0, 1.0)
                self.a2dTop = 1.0
                self.a2dBottom = -1.0
                self.a2dLeft = -aspectRatio
                self.a2dRight = aspectRatio
                # Don't forget 2dp
                if self.wantRender2dp:
                    self.aspect2dp.setScale(1.0 / aspectRatio, 1.0, 1.0)
                    self.a2dpTop = 1.0
                    self.a2dpBottom = -1.0
                    self.a2dpLeft = -aspectRatio
                    self.a2dpRight = aspectRatio

            # Reposition the aspect2d marker nodes
            self.a2dTopCenter.setPos(0, 0, self.a2dTop)
            self.a2dTopCenterNs.setPos(0, 0, self.a2dTop)
            self.a2dBottomCenter.setPos(0, 0, self.a2dBottom)
            self.a2dBottomCenterNs.setPos(0, 0, self.a2dBottom)
            self.a2dLeftCenter.setPos(self.a2dLeft, 0, 0)
            self.a2dLeftCenterNs.setPos(self.a2dLeft, 0, 0)
            self.a2dRightCenter.setPos(self.a2dRight, 0, 0)
            self.a2dRightCenterNs.setPos(self.a2dRight, 0, 0)

            self.a2dTopLeft.setPos(self.a2dLeft, 0, self.a2dTop)
            self.a2dTopLeftNs.setPos(self.a2dLeft, 0, self.a2dTop)
            self.a2dTopRight.setPos(self.a2dRight, 0, self.a2dTop)
            self.a2dTopRightNs.setPos(self.a2dRight, 0, self.a2dTop)
            self.a2dBottomLeft.setPos(self.a2dLeft, 0, self.a2dBottom)
            self.a2dBottomLeftNs.setPos(self.a2dLeft, 0, self.a2dBottom)
            self.a2dBottomRight.setPos(self.a2dRight, 0, self.a2dBottom)
            self.a2dBottomRightNs.setPos(self.a2dRight, 0, self.a2dBottom)

            # Reposition the aspect2dp marker nodes
            if self.wantRender2dp:
                self.a2dpTopCenter.setPos(0, 0, self.a2dpTop)
                self.a2dpBottomCenter.setPos(0, 0, self.a2dpBottom)
                self.a2dpLeftCenter.setPos(self.a2dpLeft, 0, 0)
                self.a2dpRightCenter.setPos(self.a2dpRight, 0, 0)

                self.a2dpTopLeft.setPos(self.a2dpLeft, 0, self.a2dpTop)
                self.a2dpTopRight.setPos(self.a2dpRight, 0, self.a2dpTop)
                self.a2dpBottomLeft.setPos(self.a2dpLeft, 0, self.a2dpBottom)
                self.a2dpBottomRight.setPos(self.a2dpRight, 0, self.a2dpBottom)

            # If anybody needs to update their GUI, put a callback on this event
            messenger.send("aspectRatioChanged")

    def userExit(self):
        # The user has requested we exit the program.  Deal with this.
        if self.exitFunc:
            self.exitFunc()
        self.notify.info("Exiting ShowBase.")
        self.finalizeExit()

    def finalizeExit(self):
        """
        Called by `userExit()` to quit the application.  The default
        implementation just calls `sys.exit()`.
        """
        sys.exit()

    # [gjeon] start wxPython
    def startWx(self, fWantWx = True):
        fWantWx = bool(fWantWx)
        if self.wantWx != fWantWx:
            self.wantWx = fWantWx
            if self.wantWx:
                self.spawnWxLoop()

    def spawnWxLoop(self):
        """ Call this method to hand the main loop over to wxPython.
        This sets up a wxTimer callback so that Panda still gets
        updated, but wxPython owns the main loop (which seems to make
        it happier than the other way around). """

        if self.wxAppCreated:
            # Don't do this twice.
            return

        init_app_for_gui()

        # Use importlib to prevent this import from being picked up
        # by modulefinder when packaging an application.
        wx = importlib.import_module('wx')

        if not self.wxApp:
            # Create a new base.wxApp.
            self.wxApp = wx.PySimpleApp(redirect = False)

        if ConfigVariableBool('wx-main-loop', True):
            # Put wxPython in charge of the main loop.  It really
            # seems to like this better; some features of wx don't
            # work properly unless this is true.

            # Set a timer to run the Panda frame 60 times per second.
            wxFrameRate = ConfigVariableDouble('wx-frame-rate', 60.0)
            self.wxTimer = wx.Timer(self.wxApp)
            self.wxTimer.Start(1000.0 / wxFrameRate.getValue())
            self.wxApp.Bind(wx.EVT_TIMER, self.__wxTimerCallback)

            # wx is now the main loop, not us any more.
            self.run = self.wxRun
            self.taskMgr.run = self.wxRun
            builtins.run = self.wxRun
            if self.appRunner:
                self.appRunner.run = self.wxRun

        else:
            # Leave Panda in charge of the main loop.  This is
            # friendlier for IDE's and interactive editing in general.
            def wxLoop(task):
                # First we need to ensure that the OS message queue is
                # processed.
                self.wxApp.Yield()

                # Now do all the wxPython events waiting on this frame.
                while self.wxApp.Pending():
                    self.wxApp.Dispatch()

                return task.again

            self.taskMgr.add(wxLoop, 'wxLoop')
        self.wxAppCreated = True

    def __wxTimerCallback(self, event):
        if Thread.getCurrentThread().getCurrentTask():
            # This happens when the wxTimer expires while igLoop is
            # rendering.  Ignore it.
            return

        self.taskMgr.step()

    def wxRun(self):
        """ This method replaces `run()` after we have called `spawnWxLoop()`.
        Since at this point wxPython now owns the main loop, this method is a
        call to wxApp.MainLoop(). """

        if Thread.getCurrentThread().getCurrentTask():
            # This happens in the p3d environment during startup.
            # Ignore it.
            return

        self.wxApp.MainLoop()

    def startTk(self, fWantTk = True):
        fWantTk = bool(fWantTk)
        if self.wantTk != fWantTk:
            self.wantTk = fWantTk
            if self.wantTk:
                self.spawnTkLoop()

    def spawnTkLoop(self):
        """ Call this method to hand the main loop over to Tkinter.
        This sets up a timer callback so that Panda still gets
        updated, but Tkinter owns the main loop (which seems to make
        it happier than the other way around). """

        if self.tkRootCreated:
            # Don't do this twice.
            return

        # Use importlib to prevent this import from being picked up
        # by modulefinder when packaging an application.
        tkinter = importlib.import_module('_tkinter')
        Pmw = importlib.import_module('Pmw')

        # Create a new Tk root.
        if not self.tkRoot:
            self.tkRoot = Pmw.initialise()
        builtins.tkroot = self.tkRoot

        init_app_for_gui()

        # Disable the Windows message loop, since Tcl wants to handle this all
        # on its own, except if the Panda window is on a separate thread.
        if self.graphicsEngine.getThreadingModel().getDrawStage() == 0:
            ConfigVariableBool('disable-message-loop', False).value = True

        if ConfigVariableBool('tk-main-loop', True):
            # Put Tkinter in charge of the main loop.  It really
            # seems to like this better; the GUI otherwise becomes
            # largely unresponsive on Mac OS X unless this is true.

            # Set a timer to run the Panda frame 60 times per second.
            tkFrameRate = ConfigVariableDouble('tk-frame-rate', 60.0)
            self.tkDelay = int(1000.0 / tkFrameRate.getValue())
            self.tkRoot.after(self.tkDelay, self.__tkTimerCallback)

            # wx is now the main loop, not us any more.
            self.run = self.tkRun
            self.taskMgr.run = self.tkRun
            builtins.run = self.tkRun
            if self.appRunner:
                self.appRunner.run = self.tkRun

        else:
            # Leave Panda in charge of the main loop.  This is
            # friendlier for IDE's and interactive editing in general.
            def tkLoop(task):
                # Do all the tkinter events waiting on this frame
                # dooneevent will return 0 if there are no more events
                # waiting or 1 if there are still more.
                # DONT_WAIT tells tkinter not to block waiting for events
                while self.tkRoot.dooneevent(tkinter.ALL_EVENTS | tkinter.DONT_WAIT):
                    pass

                return task.again

            self.taskMgr.add(tkLoop, 'tkLoop')
        self.tkRootCreated = True

    def __tkTimerCallback(self):
        if not Thread.getCurrentThread().getCurrentTask():
            self.taskMgr.step()

        self.tkRoot.after(self.tkDelay, self.__tkTimerCallback)

    def tkRun(self):
        """ This method replaces `run()` after we have called `spawnTkLoop()`.
        Since at this point Tkinter now owns the main loop, this method is a
        call to tkRoot.mainloop(). """

        if Thread.getCurrentThread().getCurrentTask():
            # This happens in the p3d environment during startup.
            # Ignore it.
            return

        self.tkRoot.mainloop()

    def startDirect(self, fWantDirect = 1, fWantTk = 1, fWantWx = 0):
        self.startTk(fWantTk)
        self.startWx(fWantWx)

        if self.wantDirect == fWantDirect:
            return

        self.wantDirect = fWantDirect

        if self.wantDirect:
            # Use importlib to prevent this import from being picked up
            # by modulefinder when packaging an application.
            DirectSession = importlib.import_module('direct.directtools.DirectSession')
            self.direct = DirectSession.DirectSession()
            self.direct.enable()
            builtins.direct = self.direct
        else:
            builtins.direct = self.direct = None

    def getRepository(self):
        return None

    def getAxes(self):
        """
        Loads and returns the ``models/misc/xyzAxis.bam`` model.

        :rtype: panda3d.core.NodePath
        """
        return loader.loadModel("models/misc/xyzAxis.bam")

    def __doStartDirect(self):
        if self.__directStarted:
            return
        self.__directStarted = False

        # Start Tk, Wx and DIRECT if specified by Config.prc
        fTk = self.config.GetBool('want-tk', 0)
        fWx = self.config.GetBool('want-wx', 0)
        # Start DIRECT if specified in Config.prc or in cluster mode
        fDirect = (self.config.GetBool('want-directtools', 0) or
                   (self.config.GetString("cluster-mode", '') != ''))
        # Set fWantTk to 0 to avoid starting Tk with this call
        self.startDirect(fWantDirect = fDirect, fWantTk = fTk, fWantWx = fWx)

    def run(self):
        """This method runs the :class:`~direct.task.Task.TaskManager`
        when ``self.appRunner is None``, which is to say, when we are
        not running from within a p3d file.  When we *are* within a p3d
        file, the Panda3D runtime has to be responsible for running the
        main loop, so we can't allow the application to do it.
        """

        if self.appRunner is None or self.appRunner.dummy or \
           (self.appRunner.interactiveConsole and not self.appRunner.initialAppImport):
            self.taskMgr.run()


    # Snake-case aliases, for people who prefer these.  We're in the process
    # of migrating everyone to use the snake-case alternatives.
    make_default_pipe = makeDefaultPipe
    make_module_pipe = makeModulePipe
    make_all_pipes = makeAllPipes
    open_window = openWindow
    close_window = closeWindow
    open_default_window = openDefaultWindow
    open_main_window = openMainWindow
    set_sleep = setSleep
    set_frame_rate_meter = setFrameRateMeter
    set_scene_graph_analyzer_meter = setSceneGraphAnalyzerMeter
    setup_window_controls = setupWindowControls
    setup_render = setupRender
    setup_render2d = setupRender2d
    setup_render2dp = setupRender2dp
    set_aspect_ratio = setAspectRatio
    get_aspect_ratio = getAspectRatio
    get_size = getSize
    make_camera = makeCamera
    make_camera2d = makeCamera2d
    make_camera2dp = makeCamera2dp
    setup_data_graph = setupDataGraph
    setup_mouse = setupMouse
    setup_mouse_cb = setupMouseCB
    enable_software_mouse_pointer = enableSoftwareMousePointer
    detach_input_device = detachInputDevice
    attach_input_device = attachInputDevice
    add_angular_integrator = addAngularIntegrator
    enable_particles = enableParticles
    disable_particles = disableParticles
    toggle_particles = toggleParticles
    create_stats = createStats
    add_sfx_manager = addSfxManager
    enable_music = enableMusic
    enable_sound_effects = enableSoundEffects
    disable_all_audio = disableAllAudio
    enable_all_audio = enableAllAudio
    init_shadow_trav = initShadowTrav
    get_background_color = getBackgroundColor
    set_background_color = setBackgroundColor
    toggle_backface = toggleBackface
    backface_culling_on = backfaceCullingOn
    backface_culling_off = backfaceCullingOff
    toggle_texture = toggleTexture
    texture_on = textureOn
    texture_off = textureOff
    toggle_wireframe = toggleWireframe
    wireframe_on = wireframeOn
    wireframe_off = wireframeOff
    disable_mouse = disableMouse
    enable_mouse = enableMouse
    silence_input = silenceInput
    revive_input = reviveInput
    set_mouse_on_node = setMouseOnNode
    change_mouse_interface = changeMouseInterface
    use_drive = useDrive
    use_trackball = useTrackball
    toggle_tex_mem = toggleTexMem
    toggle_show_vertices = toggleShowVertices
    oobe_cull = oobeCull
    show_camera_frustum = showCameraFrustum
    remove_camera_frustum = removeCameraFrustum
    save_cube_map = saveCubeMap
    save_sphere_map = saveSphereMap
    start_wx = startWx
    start_tk = startTk
    start_direct = startDirect


# A class to encapsulate information necessary for multiwindow support.
class WindowControls:
    def __init__(
            self, win, cam=None, camNode=None, cam2d=None, mouseWatcher=None,
            mouseKeyboard=None, closeCmd=lambda: 0, grid=None):
        self.win = win
        self.camera = cam
        if camNode is None and cam is not None:
            camNode = cam.node()
        self.camNode = camNode
        self.camera2d = cam2d
        self.mouseWatcher = mouseWatcher
        self.mouseKeyboard = mouseKeyboard
        self.closeCommand = closeCmd
        self.grid = grid

    def __str__(self):
        s = "window = " + str(self.win) + "\n"
        s += "camera = " + str(self.camera) + "\n"
        s += "camNode = " + str(self.camNode) + "\n"
        s += "camera2d = " + str(self.camera2d) + "\n"
        s += "mouseWatcher = " + str(self.mouseWatcher) + "\n"
        s += "mouseAndKeyboard = " + str(self.mouseKeyboard) + "\n"
        return s
