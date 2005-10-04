# This module redefines the builtin import function with one
# that prints out every import it does in a hierarchical form
# Annoying and very noisy, but sometimes useful
#import VerboseImport

from pandac.PandaModules import *

# This needs to be available early for DirectGUI imports
__builtins__["config"] = ConfigConfigureGetConfigConfigShowbase

from direct.directnotify.DirectNotifyGlobal import *
from MessengerGlobal import *
from BulletinBoardGlobal import *
from direct.task.TaskManagerGlobal import *
from EventManagerGlobal import *
from PythonUtil import *
from direct.particles.ParticleManagerGlobal import *
from PhysicsManagerGlobal import *
#from direct.interval.IntervalManager import ivalMgr
from direct.interval import IntervalManager
from InputStateGlobal import inputState

from direct.task import Task
import EventManager
import math
import sys
import Loader
import time
from direct.fsm import ClassicFSM
from direct.fsm import State
import DirectObject
import SfxPlayer
if __debug__:
    from direct.directutil import DeltaProfiler
import OnScreenDebug

__builtins__["FADE_SORT_INDEX"] = 1000
__builtins__["NO_FADE_SORT_INDEX"] = 2000

# Now ShowBase is a DirectObject.  We need this so ShowBase can hang
# hooks on messages, particularly on window-event.  This doesn't
# *seem* to cause anyone any problems.
class ShowBase(DirectObject.DirectObject):

    notify = directNotify.newCategory("ShowBase")

    def __init__(self):
        # Get the dconfig object
        #self.config = ConfigConfigureGetConfigConfigShowbase
        self.config = config
        # Setup wantVerifyPdb as soon as reasonable:
        Verify.wantVerifyPdb = self.config.GetBool('want-verify-pdb', 0)

        self.printEnvDebugInfo()

        if self.config.GetBool('use-vfs', 1):
            vfs = VirtualFileSystem.getGlobalPtr()
        else:
            vfs = None

        self.nextWindowIndex = 1

        # Store dconfig variables
        self.sfxActive = self.config.GetBool('audio-sfx-active', 1)
        self.musicActive = self.config.GetBool('audio-music-active', 1)
        self.wantFog = self.config.GetBool('want-fog', 1)

        self.screenshotExtension = self.config.GetString('screenshot-extension', 'jpg')
        self.musicManager = None
        self.musicManagerIsValid = None
        self.sfxManagerList = []
        self.sfxManagerIsValidList = []

        self.wantStats = self.config.GetBool('want-pstats', 0)

        self.clientSleep = self.config.GetFloat('client-sleep', 0.)
        # magic-word override
        self.mwClientSleep = 0.

        # using 'sleep' once per frame to limit CPU usage.
        self.sleepCycle = 0.0

        # Fill this in with a function to invoke when the user "exits"
        # the program by closing the main window.
        self.exitFunc = None

        Task.TaskManager.taskTimerVerbose = self.config.GetBool('task-timer-verbose', 0)
        Task.TaskManager.extendedExceptions = self.config.GetBool('extended-exceptions', 0)
        Task.TaskManager.pStatsTasks = self.config.GetBool('pstats-tasks', 0)

        # Set up the TaskManager to reset the PStats clock back
        # whenever we resume from a pause.  This callback function is
        # a little hacky, but we can't call it directly from within
        # the TaskManager because he doesn't know about PStats (and
        # has to run before libpanda is even loaded).
        taskMgr.resumeFunc = PStatClient.resumeAfterPause

        fsmRedefine = self.config.GetBool('fsm-redefine', 0)
        State.FsmRedefine = fsmRedefine

        # If the aspect ratio is 0 or None, it means to infer the
        # aspect ratio from the window size.
        self.aspectRatio = ConfigVariableDouble('aspect-ratio', 0).getValue()

        self.windowType = self.config.GetString('window-type', 'onscreen')
        self.requireWindow = self.config.GetBool('require-window', 1)

        # base.win is the main, or only window; base.winList is a list of
        # *all* windows.  Similarly with base.camList.
        self.win = None
        self.frameRateMeter = None
        self.winList = []
        self.winControls = []
        self.mainWinMinimized = 0
        self.pipe = None
        self.pipeList = []
        self.mouse2cam = None
        self.buttonThrowers = None
        self.mouseWatcher = None
        self.mouseWatcherNode = None
        self.pointerWatcherNodes = None
        self.mouseInterface = None
        self.drive = None
        self.trackball = None
        self.cam = None
        self.camList = []
        self.camNode = None
        self.camLens = None
        self.camera = None
        self.camera2d = None
        self.camera2dp = None
        self.camFrustumVis = None

        # This is used for syncing multiple PCs in a distributed cluster
        try:
            # Has the cluster sync variable been set externally?
            self.clusterSyncFlag = clusterSyncFlag
        except NameError:
            # Has the clusterSyncFlag been set via a config variable
            self.clusterSyncFlag = self.config.GetBool('cluster-sync', 0)

        self.hidden = NodePath('hidden')

        # We need a graphics engine to manage the actual rendering.
        self.graphicsEngine = GraphicsEngine()

        self.setupRender()
        self.setupRender2d()
        self.setupRender2dp()
        self.setupDataGraph()

        # This is a placeholder for a CollisionTraverser.  If someone
        # stores a CollisionTraverser pointer here, we'll traverse it
        # in the collisionLoop task.
        self.shadowTrav = 0
        # in the collisionLoop task.
        self.cTrav = 0
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
            import random, whrandom

            seed = self.recorder.getRandomSeed()
            random.seed(seed)
            whrandom.seed(seed & 0xff, (seed >> 8) & 0xff, (seed >> 16) & 0xff)

        # Now that we've set up the window structures, assign an exitfunc.
        self.oldexitfunc = getattr(sys, 'exitfunc', None)
        sys.exitfunc = self.exitfunc

        # Open the default rendering window.
        if self.windowType != 'none':
            self.openDefaultWindow()

        self.loader = Loader.Loader(self)
        self.eventMgr = eventMgr
        self.messenger = messenger
        self.bboard = bulletinBoard
        self.taskMgr = taskMgr

        # Particle manager
        self.particleMgr = particleMgr
        self.particleMgr.setFrameStepping(1)
        self.particleMgrEnabled = 0

        # Physics manager
        self.physicsMgr = physicsMgr
        integrator = LinearEulerIntegrator()
        self.physicsMgr.attachLinearIntegrator(integrator)
        self.physicsMgrEnabled = 0
        self.physicsMgrAngular = 0

        self.createBaseAudioManagers()
        self.createStats()

        self.AppHasAudioFocus = 1

        __builtins__["base"] = self
        __builtins__["render2d"] = self.render2d
        __builtins__["render2dp"] = self.render2dp
        __builtins__["aspect2d"] = self.aspect2d
        __builtins__["aspect2dp"] = self.aspect2dp
        __builtins__["render"] = self.render
        __builtins__["hidden"] = self.hidden
        __builtins__["camera"] = self.camera
        __builtins__["loader"] = self.loader
        __builtins__["taskMgr"] = self.taskMgr
        __builtins__["eventMgr"] = self.eventMgr
        __builtins__["messenger"] = self.messenger
        __builtins__["bboard"] = self.bboard
        # Config needs to be defined before ShowBase is constructed
        #__builtins__["config"] = self.config
        __builtins__["run"] = self.run
        __builtins__["ostream"] = Notify.out()
        __builtins__["directNotify"] = directNotify
        __builtins__["globalClock"] = ClockObject.getGlobalClock()
        __builtins__["vfs"] = vfs
        __builtins__["cpMgr"] = ConfigPageManager.getGlobalPtr()
        __builtins__["cvMgr"] = ConfigVariableManager.getGlobalPtr()
        __builtins__["pandaSystem"] = PandaSystem.getGlobalPtr()
        __builtins__["__dev__"] = base.config.GetBool('want-dev', 0)
        __builtins__["wantUberdog"] = base.config.GetBool('want-uberdog', 1)
        if __debug__:
            __builtins__["deltaProfiler"] = DeltaProfiler.DeltaProfiler("ShowBase")
        __builtins__["onScreenDebug"] = OnScreenDebug.OnScreenDebug()

        ShowBase.notify.info('__dev__ == %s' % __dev__)

        # Now hang a hook on the window-event from Panda.  This allows
        # us to detect when the user resizes, minimizes, or closes the
        # main window.
        self.accept('window-event', self.__windowEvent)

        # Transition effects (fade, iris, etc)
        import Transitions
        self.transitions = Transitions.Transitions(self.loader)

        # Setup the window controls - handy for multiwindow applications
        self.setupWindowControls()

        # Start Tk and DIRECT if specified by Config.prc
        fTk = self.config.GetBool('want-tk', 0)
        # Start DIRECT if specified in Config.prc or in cluster mode
        fDirect = (self.config.GetBool('want-directtools', 0) or
                   (base.config.GetString("cluster-mode", '') != ''))
        # Set fWantTk to 0 to avoid starting Tk with this call
        self.startDirect(fWantDirect = fDirect, fWantTk = fTk)
        # Start IGLOOP
        self.restart()

    def printEnvDebugInfo(self):
        """
        Print some information about the environment that we are running
        in.  Stuff like the model paths and other paths.  Feel free to
        add stuff to this.
        """
        if self.config.GetBool('want-env-debug-info', 0):
           print "\n\nEnvironment Debug Info {"
           print "* model path:"
           print getModelPath()
           print "* texture path:"
           print getTexturePath()
           print "* sound path:"
           print getSoundPath()
           print "}"

    def exitfunc(self):
        """
        This should be assigned to sys.exitfunc to be called just
        before Python shutdown.  It guarantees that the Panda window
        is closed cleanly, so that we free system resources, restore
        the desktop and keyboard functionality, etc.
        """
        self.graphicsEngine.removeAllWindows()

        if self.musicManager:
            self.musicManager.shutdown()

        del self.win
        del self.winList
        del self.pipe
        del self.musicManager
        del self.sfxManagerList

        try:
            direct.panel.destroy()
        except StandardError:
            pass

        if self.oldexitfunc:
            self.oldexitfunc()

    def makeDefaultPipe(self):
        """
        Creates the default GraphicsPipe, which will be used to make
        windows unless otherwise specified.
        """
        assert(self.pipe == None)
        selection = GraphicsPipeSelection.getGlobalPtr()
        selection.printPipeTypes()
        self.pipe = selection.makeDefaultPipe()
        if not self.pipe:
            self.notify.error(
                "No graphics pipe is available!\n"
                "Your Config.prc file must name at least one valid panda display\n"
                "library via load-display or aux-display.")

        self.notify.info("Default graphics pipe is %s (%s)." % (self.pipe.getInterfaceName(), self.pipe.getType().getName()))
        self.pipeList.append(self.pipe)

    def makeAllPipes(self):
        """
        Creates all GraphicsPipes that the system knows about and fill up
        self.pipeList with them.
        """
        shouldPrintPipes = 0
        selection = GraphicsPipeSelection.getGlobalPtr()
        selection.loadAuxModules()

        # First, we should make sure the default pipe exists.
        if self.pipe == None:
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
                    self.notify.info("Got aux graphics pipe %s (%s)." % (pipe.getInterfaceName(), pipe.getType().getName()))
                    self.pipeList.append(pipe)
                else:
                    self.notify.info("Could not make graphics pipe %s." % (pipeType.getName()))

    def openWindow(self, props = None, pipe = None, gsg = None,
                   type = None, name = None, scene = None,
                   size = None, aspectRatio = None):
        """
        Creates a window and adds it to the list of windows that are
        to be updated every frame.
        """
        if pipe == None:
            pipe = self.pipe

            if pipe == None:
                self.makeDefaultPipe()
                pipe = self.pipe

            if pipe == None:
                # We couldn't get a pipe.
                return None

        if gsg == None:
            # If we weren't given a gsg, create a new one just for
            # this window.
            gsg = self.graphicsEngine.makeGsg(pipe)

            if gsg == None:
                # Couldn't make a gsg.
                return None

        if type == None:
            type = self.windowType

        if props == None:
            props = WindowProperties.getDefault()

        if size != None:
            # If we were given an explicit size, use it; otherwise,
            # the size from the properties is used.
            props = WindowProperties(props)
            props.setSize(size[0], size[1])

        if name == None:
            name = 'window%s' % (self.nextWindowIndex)
            self.nextWindowIndex += 1

        win = None
        if type == 'onscreen':
            win = self.graphicsEngine.makeWindow(gsg, name, 0)
        elif type == 'offscreen':
            win = self.graphicsEngine.makeBuffer(
                gsg, name, 0, props.getXSize(), props.getYSize())
            
        if win == None:
            # Couldn't create a window!
            return None

        if hasattr(win, "requestProperties"):
            win.requestProperties(props)

        if self.win == None:
            self.win = win

        self.winList.append(win)

        # Set up a 3-d camera for the window by default.
        self.makeCamera(win, scene = scene, aspectRatio = aspectRatio)

        return win

    def closeWindow(self, win):
        """
        Closes the indicated window and removes it from the list of
        windows.  If it is the main window, clears the main window
        pointer to None.
        """
        # First, remove all of the cameras associated with display
        # regions on the window.
        numRegions = win.getNumDisplayRegions()
        for i in range(numRegions):
            dr = win.getDisplayRegion(i)
            cam = NodePath(dr.getCamera())
            dr.setCamera(NodePath())

            if not cam.isEmpty() and cam.node().getNumDisplayRegions() == 0:
                # If the camera is used by no other DisplayRegions,
                # remove it.
                if self.camList.count(cam) != 0:
                    self.camList.remove(cam)

                # Don't throw away self.camera; we want to
                # preserve it for reopening the window.
                if cam == self.cam:
                    self.cam = None
                cam.removeNode()

        # Now we can actually close the window.
        self.graphicsEngine.removeWindow(win)
        self.winList.remove(win)

        if win == self.win:
            self.win = None
            self.frameRateMeter = None

    def openDefaultWindow(self):
        # Creates the main window for the first time, without being
        # too particular about the kind of graphics API that is
        # chosen.  The suggested window type from the load-display
        # config variable is tried first; if that fails, the first
        # window type that can be successfully opened at all is
        # accepted.  Returns true on success, false otherwise.
        #
        # This is intended to be called only once, at application
        # startup.  It is normally called automatically unless
        # window-type is configured to 'none'.

        self.openMainWindow()

        # Give the window a chance to truly open.
        self.graphicsEngine.openWindows()
        if self.win != None and not self.isMainWindowOpen():
            self.notify.info("Window did not open, removing.")
            self.closeWindow(self.win)

        if self.win == None:
            # Try a little harder if the window wouldn't open.
            self.makeAllPipes()
            while self.win == None and len(self.pipeList) > 1:
                self.pipeList.remove(self.pipe)
                self.pipe = self.pipeList[0]
                self.openMainWindow()

                self.graphicsEngine.openWindows()
                if self.win != None and not self.isMainWindowOpen():
                    self.notify.info("Window did not open, removing.")
                    self.closeWindow(self.win)

        if self.win == None:
            self.notify.warning("Unable to open '%s' window." % (self.windowType))
            if self.requireWindow:
                # Unless require-window is set to false, it is an
                # error not to open a window.
                raise StandardError, 'Could not open window.'

        return self.win != None

    def isMainWindowOpen(self):
        if self.win != None:
            return self.win.isValid()
        return 0

    def openMainWindow(self, *args, **kw):
        """
        Creates the initial, main window for the application, and sets
        up the mouse and render2d structures appropriately for it.  If
        this method is called a second time, it will close the
        previous main window and open a new one, preserving the lens
        properties in base.camLens.

        The return value is true on success, or false on failure (in
        which case base.win may be either None, or the previous,
        closed window).
        """
        success = 1
        oldWin = self.win
        oldLens = self.camLens
        oldClearColorActive = None
        if self.win != None:
            # Close the previous window.
            oldClearColorActive = self.win.getClearColorActive()
            oldClearColor = VBase4(self.win.getClearColor())
            oldClearDepthActive = self.win.getClearDepthActive()
            oldClearDepth = self.win.getClearDepth()
            self.closeWindow(self.win)

        # Open a new window.
        self.openWindow(*args, **kw)
        if self.win == None:
            self.win = oldWin
            self.winList.append(oldWin)
            success = 0

        if self.win != None:
            if isinstance(self.win, GraphicsWindow):
                self.setupMouse(self.win)
            self.makeCamera2d(self.win)
            self.makeCamera2dp(self.win)

            if oldLens != None:
                # Restore the previous lens properties.
                self.camNode.setLens(oldLens)
                self.camLens = oldLens

            if oldClearColorActive != None:
                # Restore the previous clear properties.
                self.win.setClearColorActive(oldClearColorActive)
                self.win.setClearColor(oldClearColor)
                self.win.setClearDepthActive(oldClearDepthActive)
                self.win.setClearDepth(oldClearDepth)

            self.setFrameRateMeter(self.config.GetBool('show-frame-rate-meter', 0))

        return success

    def setSleep(self, amount):
        """
        Sets up a task that calls python 'sleep' every frame.  This is a simple
        way to reduce the CPU usage (and frame rate) of a panda program.
        """
        if (self.sleepCycle == amount): return()
        if (time == 0.0):
          self.taskMgr.remove('sleep-cycle')
        else:
          self.sleepCycle = amount
          self.taskMgr.add(self.sleepCycleTask, 'sleep-cycle')

    def sleepCycleTask(self, state):
        time.sleep(self.sleepCycle)
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

    def setupWindowControls(self):
        if not self.winControls:
            winCtrl = WindowControls(self.win, mouseWatcher=self.mouseWatcher,
                                     cam=self.camera, cam2d=self.camera2d,
                                     mouseKeyboard = self.dataRoot.find("**/*"))
            self.winControls.append(winCtrl)


    def setupRender(self):
        """
        Creates the render scene graph, the primary scene graph for
        rendering 3-d geometry.
        """
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
        self.render2d = NodePath('render2d')

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

        # The normal 2-d DisplayRegion has an aspect ratio that
        # matches the window, but its coordinate system is square.
        # This means anything we parent to render2d gets stretched.
        # For things where that makes a difference, we set up
        # aspect2d, which scales things back to the right aspect
        # ratio.
        aspectRatio = self.getAspectRatio()
        self.aspect2d = self.render2d.attachNewNode(PGTop("aspect2d"))
        self.aspect2d.setScale(1.0 / aspectRatio, 1.0, 1.0)

        # It's important to know the bounds of the aspect2d screen.
        self.a2dTop = 1.0
        self.a2dBottom = -1.0
        self.a2dLeft = -aspectRatio
        self.a2dRight = aspectRatio

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

        # We used to avoid clearing the depth buffer before drawing
        # render2d, but nowadays we clear it anyway, since we
        # occasionally want to put 3-d geometry under render2d, and
        # it's simplest (and seems to be easier on graphics drivers)
        # if the 2-d scene has been cleared first.

        dt = DepthTestAttrib.make(DepthTestAttrib.MNone)
        dw = DepthWriteAttrib.make(DepthWriteAttrib.MOff)
        self.render2dp.setDepthTest(0)
        self.render2dp.setDepthWrite(0)

        self.render2dp.setMaterialOff(1)
        self.render2dp.setTwoSided(1)

        # The normal 2-d DisplayRegion has an aspect ratio that
        # matches the window, but its coordinate system is square.
        # This means anything we parent to render2d gets stretched.
        # For things where that makes a difference, we set up
        # aspect2d, which scales things back to the right aspect
        # ratio.

        aspectRatio = self.getAspectRatio()
        self.aspect2dp = self.render2dp.attachNewNode(PGTop("aspect2dp"))
        self.aspect2dp.node().setStartSort(16384)
        self.aspect2dp.setScale(1.0 / aspectRatio, 1.0, 1.0)

        # It's important to know the bounds of the aspect2d screen.
        self.a2dpTop = 1.0
        self.a2dpBottom = -1.0
        self.a2dpLeft = -aspectRatio
        self.a2dpRight = aspectRatio

    def getAspectRatio(self, win = None):
        # Returns the actual aspect ratio of the indicated (or main
        # window), or the default aspect ratio if there is not yet a
        # main window.

        if self.aspectRatio:
            return self.aspectRatio

        aspectRatio = 1

        if win == None:
            win = self.win

        if win != None and win.hasSize():
            aspectRatio = float(win.getXSize()) / float(win.getYSize())

        else:
            if win == None:
                props = WindowProperties.getDefault()
            else:
                props = win.getRequestedProperties()
                if not props.hasSize():
                    props = WindowProperties.getDefault()
                    
            if props.hasSize():
                aspectRatio = float(props.getXSize()) / float(props.getYSize())

        return aspectRatio

    def makeCamera(self, win, sort = 0, scene = None,
                   displayRegion = (0, 1, 0, 1), aspectRatio = None,
                   lens = None, camName = 'cam'):
        """
        Makes a new 3-d camera associated with the indicated window,
        and creates a display region in the indicated subrectangle.
        """
        dr = win.makeDisplayRegion(*displayRegion)
        dr.setSort(sort)

        if scene == None:
            scene = self.render

        # By default, we do not clear 3-d display regions (the entire
        # window will be cleared, which is normally sufficient).

        # Now make a new Camera node.
        camNode = Camera(camName)
        if lens == None:
            lens = PerspectiveLens()

            if aspectRatio == None:
                aspectRatio = self.getAspectRatio(win)
            lens.setAspectRatio(aspectRatio)

        camNode.setLens(lens)

        # self.camera is the parent node of all cameras: a node that
        # we can move around to move all cameras as a group.
        if self.camera == None:
            self.camera = self.render.attachNewNode('camera')
            __builtins__["camera"] = self.camera

        cam = self.camera.attachNewNode(camNode)
        dr.setCamera(cam)

        if self.cam == None:
            self.cam = cam
            self.camNode = camNode
            self.camLens = lens

        self.camList.append(cam)

        return cam

    def makeCamera2d(self, win, sort = 10,
                     displayRegion = (0, 1, 0, 1), coords = (-1, 1, -1, 1),
                     lens = None):
        """
        Makes a new camera2d associated with the indicated window, and
        assigns it to render the indicated subrectangle of render2d.
        """
        dr = win.makeDisplayRegion(*displayRegion)
        dr.setSort(sort)

        # Enable clearing of the depth buffer on this new display
        # region (see the comment in setupRender2d, above).
        dr.setClearDepthActive(1)

        left, right, bottom, top = coords

        # Now make a new Camera node.
        cam2dNode = Camera('cam2d')
        if lens == None:
            lens = OrthographicLens()
            lens.setFilmSize(right - left, top - bottom)
            lens.setFilmOffset((right + left) * 0.5, (top + bottom) * 0.5)
            lens.setNearFar(-1000, 1000)
        cam2dNode.setLens(lens)

        # self.camera2d is the analog of self.camera, although it's
        # not as clear how useful it is.
        if self.camera2d == None:
            self.camera2d = self.render2d.attachNewNode('camera2d')

        camera2d = self.camera2d.attachNewNode(cam2dNode)
        dr.setCamera(camera2d)

        return camera2d

    def makeCamera2dp(self, win, sort = 20,
                      displayRegion = (0, 1, 0, 1), coords = (-1, 1, -1, 1),
                      lens = None):
        """
        Makes a new camera2dp associated with the indicated window, and
        assigns it to render the indicated subrectangle of render2dp.
        """
        dr = win.makeDisplayRegion(*displayRegion)
        dr.setSort(sort)

        # Enable clearing of the depth buffer on this new display
        # region (see the comment in setupRender2d, above).
        dr.setClearDepthActive(1)

        left, right, bottom, top = coords

        # Now make a new Camera node.
        cam2dNode = Camera('cam2d')
        if lens == None:
            lens = OrthographicLens()
            lens.setFilmSize(right - left, top - bottom)
            lens.setFilmOffset((right + left) * 0.5, (top + bottom) * 0.5)
            lens.setNearFar(-1000, 1000)
        cam2dNode.setLens(lens)

        # self.camera2d is the analog of self.camera, although it's
        # not as clear how useful it is.
        if self.camera2dp == None:
            self.camera2dp = self.render2dp.attachNewNode('camera2dp')

        camera2dp = self.camera2dp.attachNewNode(cam2dNode)
        dr.setCamera(camera2dp)

        return camera2dp


    def setupDataGraph(self):
        """
        Creates the data graph and populates it with the basic input
        devices.
        """
        self.dataRoot = NodePath('dataRoot')
        # Cache the node so we do not ask for it every frame
        self.dataRootNode = self.dataRoot.node()
        self.dataUnused = NodePath('dataUnused')


    def setupMouse(self, win):
        """
        Creates the structures necessary to monitor the mouse input,
        using the indicated window.  If the mouse has already been set
        up for a different window, those structures are deleted first.
        """

        if self.buttonThrowers != None:
            for bt in self.buttonThrowers:
                mw = bt.getParent()
                mk = mw.getParent()
                bt.removeNode()
                mw.removeNode()
                mk.removeNode()

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

        self.buttonThrowers = []
        self.pointerWatcherNodes = []
        for i in range(win.getNumInputDevices()):
            name = win.getInputDeviceName(i)
            mk = self.dataRoot.attachNewNode(MouseAndKeyboard(win, i, name))
            mw = mk.attachNewNode(MouseWatcher(name))
            mb = mw.node().getModifierButtons()
            mb.addButton(KeyboardButton.shift())
            mb.addButton(KeyboardButton.control())
            mb.addButton(KeyboardButton.alt())
            mw.node().setModifierButtons(mb)
            bt = mw.attachNewNode(ButtonThrower(name))
            if (i != 0):
                bt.node().setPrefix('mousedev'+str(i)+'-')
            mods = ModifierButtons()
            mods.addButton(KeyboardButton.shift())
            mods.addButton(KeyboardButton.control())
            mods.addButton(KeyboardButton.alt())
            bt.node().setModifierButtons(mods)
            self.buttonThrowers.append(bt)
            if (win.hasPointer(i)):
                self.pointerWatcherNodes.append(mw.node())

        self.mouseWatcher = self.buttonThrowers[0].getParent()
        self.mouseWatcherNode = self.mouseWatcher.node()
        # print "ButtonThrowers = ",self.buttonThrowers
        # print "PointerWatcherNodes = ",self.pointerWatcherNodes

        if self.recorder:
            # If we have a recorder, the mouseWatcher belongs under a
            # special MouseRecorder node, which may intercept the
            # mouse activity.
            mw = self.buttonThrowers[0].getParent()
            mouseRecorder = MouseRecorder('mouse')
            self.recorder.addRecorder('mouse', mouseRecorder.upcastToRecorderBase())
            np = mw.getParent().attachNewNode(mouseRecorder)
            mw.reparentTo(np)

        # Now we have the main trackball & drive interfaces.
        # useTrackball() and useDrive() switch these in and out; only
        # one is in use at a given time.
        self.trackball = self.dataUnused.attachNewNode(Trackball('trackball'))
        self.drive = self.dataUnused.attachNewNode(DriveInterface('drive'))
        self.mouse2cam = self.dataUnused.attachNewNode(Transform2SG('mouse2cam'))
        self.mouse2cam.node().setNode(self.camera.node())

        # The default is trackball mode, which is more convenient for
        # ad-hoc development in Python using ShowBase.  Applications
        # can explicitly call base.useDrive() if they prefer a drive
        # interface.
        self.mouseInterface = self.trackball
        self.useTrackball()

        # A special ButtonThrower to generate keyboard events and
        # include the time from the OS.  This is separate only to
        # support legacy code that did not expect a time parameter; it
        # will eventually be folded into the normal ButtonThrower,
        # above.
        mw = self.buttonThrowers[0].getParent()
        self.timeButtonThrower = mw.attachNewNode(ButtonThrower('timeButtons'))
        self.timeButtonThrower.node().setPrefix('time-')
        self.timeButtonThrower.node().setTimeFlag(1)

        # Tell the gui system about our new mouse watcher.
        self.aspect2d.node().setMouseWatcher(mw.node())
        self.aspect2dp.node().setMouseWatcher(mw.node())
        mw.node().addRegion(PGMouseWatcherBackground())

    def enableSoftwareMousePointer(self):
        """enableSoftwareMousePointer(self)

        Creates some geometry and parents it to render2d to show
        the currently-known mouse position.  Useful if the mouse
        pointer is invisible for some reason.
        """
        mouseViz = render2d.attachNewNode('mouseViz')
        lilsmiley = loader.loadModel('lilsmiley')
        lilsmiley.reparentTo(mouseViz)

        aspectRatio = self.getAspectRatio()
        # Scale the smiley face to 32x32 pixels.
        height = self.win.getYSize()
        lilsmiley.setScale(
            32.0 / height / aspectRatio,
            1.0, 32.0 / height)
        self.mouseWatcherNode.setGeometry(mouseViz.node())

    def getAlt(self):
        return self.mouseWatcherNode.getModifierButtons().isDown(
            KeyboardButton.alt())

    def getShift(self):
        return self.mouseWatcherNode.getModifierButtons().isDown(
            KeyboardButton.shift())

    def getControl(self):
        return self.mouseWatcherNode.getModifierButtons().isDown(
            KeyboardButton.control())

    def addAngularIntegrator(self):
        """addAngularIntegrator(self)"""
        if (self.physicsMgrAngular == 0):
            self.physicsMgrAngular = 1
            integrator = AngularEulerIntegrator()
            self.physicsMgr.attachAngularIntegrator(integrator)

    def enableParticles(self):
        """enableParticles(self)"""
        self.particleMgrEnabled = 1
        self.physicsMgrEnabled = 1
        self.taskMgr.remove('manager-update')
        self.taskMgr.add(self.updateManagers, 'manager-update')

    def disableParticles(self):
        """enableParticles(self)"""
        self.particleMgrEnabled = 0
        self.physicsMgrEnabled = 0
        self.taskMgr.remove('manager-update')

    def toggleParticles(self):
        if self.particleMgrEnabled == 0:
            self.enableParticles()
        else:
            self.disableParticles()

    def isParticleMgrEnabled(self):
        return self.particleMgrEnabled

    def isPhysicsMgrEnabled(self):
        return self.physicsMgrEnabled

    def updateManagers(self, state):
        dt = globalClock.getDt()
        if (self.particleMgrEnabled == 1):
            self.particleMgr.doParticles(dt)
        if (self.physicsMgrEnabled == 1):
            self.physicsMgr.doPhysics(dt)
        return Task.cont

    def createStats(self):
        # You must specify a pstats-host in your Config.prc
        # The default is localhost
        if self.wantStats:
            PStatClient.connect()

    def addSfxManager(self, extraSfxManager):
        # keep a list of sfx manager objects to apply settings to, since there may be others
        # in addition to the one we create here
        self.sfxManagerList.append(extraSfxManager)
        newSfxManagerIsValid = (extraSfxManager!=None) and extraSfxManager.isValid()
        self.sfxManagerIsValidList.append(newSfxManagerIsValid)
        if newSfxManagerIsValid:
            extraSfxManager.setActive(self.sfxActive)

    def createBaseAudioManagers(self):
        self.sfxPlayer = SfxPlayer.SfxPlayer()
        sfxManager = AudioManager.createAudioManager()
        self.addSfxManager(sfxManager)

        self.musicManager = AudioManager.createAudioManager()
        self.musicManagerIsValid=self.musicManager!=None \
                and self.musicManager.isValid()
        if self.musicManagerIsValid:
            # ensure only 1 midi song is playing at a time:
            self.musicManager.setConcurrentSoundLimit(1)
            self.musicManager.setActive(self.musicActive)

    # enableMusic/enableSoundEffects are meant to be called in response to a user request
    # so sfxActive/musicActive represent how things *should* be, regardless of App/OS/HW state
    def enableMusic(self, bEnableMusic):
        # dont setActive(1) if no audiofocus
        if self.AppHasAudioFocus and self.musicManagerIsValid:
            self.musicManager.setActive(bEnableMusic)
        self.musicActive = bEnableMusic
        if bEnableMusic:
            self.notify.debug("Enabling music")
        else:
            self.notify.debug("Disabling music")

    def SetAllSfxEnables(self, bEnabled):
        for i in range(len(self.sfxManagerList)):
            if (self.sfxManagerIsValidList[i]):
                self.sfxManagerList[i].setActive(bEnabled)

    def enableSoundEffects(self, bEnableSoundEffects):
        # dont setActive(1) if no audiofocus
        if self.AppHasAudioFocus or (bEnableSoundEffects==0):
            self.SetAllSfxEnables(bEnableSoundEffects)
        self.sfxActive=bEnableSoundEffects
        if bEnableSoundEffects:
            self.notify.debug("Enabling sound effects")
        else:
            self.notify.debug("Disabling sound effects")

    # enable/disableAllAudio allow a programmable global override-off for current audio settings.
    # they're meant to be called when app loses audio focus (switched out), so we can turn off sound
    # without affecting internal sfxActive/musicActive sound settings, so things
    # come back ok when the app is switched back to

    def disableAllAudio(self):
        self.AppHasAudioFocus = 0
        self.SetAllSfxEnables(0)
        if self.musicManagerIsValid:
            self.musicManager.setActive(0)
        self.notify.debug("Disabling audio")

    def enableAllAudio(self):
        self.AppHasAudioFocus = 1
        self.SetAllSfxEnables(self.sfxActive)
        if self.musicManagerIsValid:
            self.musicManager.setActive(self.musicActive)
        self.notify.debug("Enabling audio")

    # This function should only be in the loader but is here for
    # backwards compatibility. Please do not add code here, add
    # it to the loader.
    def loadSfx(self, name):
        return self.loader.loadSfx(name)

    # This function should only be in the loader but is here for
    # backwards compatibility. Please do not add code here, add
    # it to the loader.
    def loadMusic(self, name):
        return self.loader.loadMusic(name)

    def playSfx(self, sfx, looping = 0, interrupt = 1, volume = None, time = 0.0, node = None):
        # This goes through a special player for potential localization
        return self.sfxPlayer.playSfx(sfx, looping, interrupt, volume, time, node)

    def playMusic(self, music, looping = 0, interrupt = 1, volume = None, time = 0.0):
        if music:
            if volume != None:
                music.setVolume(volume)

            # if interrupt was set to 0, start over even if it's already playing
            if interrupt or (music.status() != AudioSound.PLAYING):
                music.setTime(time)
                music.setLoop(looping)
                music.play()

    def resetPrevTransform(self, state):
        # Clear out the previous velocity deltas now, after we have
        # rendered (the previous frame).  We do this after the render,
        # so that we have a chance to draw a representation of spheres
        # along with their velocities.  At the beginning of the frame
        # really means after the command prompt, which allows the user
        # to interactively query these deltas meaningfully.

        if self.cTrav:
            self.cTrav.resetPrevTransform(self.render)
        return Task.cont

    def dataLoop(self, state):
        # traverse the data graph.  This reads all the control
        # inputs (from the mouse and keyboard, for instance) and also
        # directly acts upon them (for instance, to move the avatar).
        self.dgTrav.traverse(self.dataRootNode)
        return Task.cont

    def ivalLoop(self, state):
        # Execute all intervals in the global ivalMgr.
        IntervalManager.ivalMgr.step()
        return Task.cont

    def shadowCollisionLoop(self, state):
        # run the collision traversal if we have a
        # CollisionTraverser set.
        if self.shadowTrav:
            self.shadowTrav.traverse(self.render)
        return Task.cont

    def collisionLoop(self, state):
        # run the collision traversal if we have a
        # CollisionTraverser set.
        if self.cTrav:
            self.cTrav.traverse(self.render)
        if self.appTrav:
            self.appTrav.traverse(self.render)
        return Task.cont

    def igLoop(self, state):
        # We render the watch variables for the onScreenDebug as soon
        # as we reasonably can before the renderFrame().
        onScreenDebug.render()

        if self.recorder:
            self.recorder.recordFrame()

        # Finally, render the frame.
        self.graphicsEngine.renderFrame()
        if self.clusterSyncFlag:
            self.graphicsEngine.syncFrame()

        # We clear the text buffer for the onScreenDebug as soon
        # as we reasonably can after the renderFrame().
        onScreenDebug.clear()

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
        else:
            # magic word overrides config
            if self.mwClientSleep:
                time.sleep(self.mwClientSleep)
            elif self.clientSleep:
                time.sleep(self.clientSleep)

        # Lerp stuff needs this event, and it must be generated in
        # C++, not in Python.
        throwNewFrame()
        return Task.cont

    def restart(self):
        self.shutdown()
        # resetPrevTransform goes at the very beginning of the frame.
        self.taskMgr.add(self.resetPrevTransform, 'resetPrevTransform', priority = -51)
        # give the dataLoop task a reasonably "early" priority,
        # so that it will get run before most tasks
        self.taskMgr.add(self.dataLoop, 'dataLoop', priority = -50)
        # spawn the ivalLoop with a later priority, so that it will
        # run after most tasks, but before igLoop.
        self.taskMgr.add(self.ivalLoop, 'ivalLoop', priority = 20)
        # make the collisionLoop task run before igLoop,
        # but leave enough room for the app to insert tasks
        # between collisionLoop and igLoop
        self.taskMgr.add(self.collisionLoop, 'collisionLoop', priority = 30)
        # do the shadowCollisionLoop after the collisionLoop and
        # befor the igLoop:
        self.taskMgr.add(self.shadowCollisionLoop, 'shadowCollisionLoop', priority = 45)
        # give the igLoop task a reasonably "late" priority,
        # so that it will get run after most tasks
        self.taskMgr.add(self.igLoop, 'igLoop', priority = 50)
        self.eventMgr.restart()

    def shutdown(self):
        self.taskMgr.remove('igLoop')
        self.taskMgr.remove('shadowCollisionLoop')
        self.taskMgr.remove('collisionLoop')
        self.taskMgr.remove('dataLoop')
        self.taskMgr.remove('resetPrevTransform')
        self.taskMgr.remove('ivalLoop')
        self.eventMgr.shutdown()

    def getBackgroundColor(self, win = None):
        """
        Returns the current window background color.  This assumes
        the window is set up to clear the color each frame (this is
        the normal setting).
        """
        if win == None:
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
        if g != None:
            color = VBase4(r, g, b, a)
        else:
            arg = r
            if isinstance(arg, VBase4):
                color = arg
            else:
                color = VBase4(arg[0], arg[1], arg[2], a)

        if win == None:
            win = self.win

        if win:
           win.setClearColor(color)

    def toggleBackface(self):
        if self.backfaceCullingEnabled:
            self.backfaceCullingOff()
        else:
            self.backfaceCullingOn()

    def backfaceCullingOn(self):
        if not self.backfaceCullingEnabled:
            self.render.setTwoSided(0)
        self.backfaceCullingEnabled = 1

    def backfaceCullingOff(self):
        if self.backfaceCullingEnabled:
            self.render.setTwoSided(1)
        self.backfaceCullingEnabled = 0

    def toggleTexture(self):
        if self.textureEnabled:
            self.textureOff()
        else:
            self.textureOn()

    def textureOn(self):
        self.render.clearTexture()
        self.textureEnabled = 1

    def textureOff(self):
        self.render.setTextureOff(100)
        self.textureEnabled = 0

    def toggleWireframe(self):
        if self.wireframeEnabled:
            self.wireframeOff()
        else:
            self.wireframeOn()

    def wireframeOn(self):
        self.render.setRenderModeWireframe(100)
        self.render.setTwoSided(1)
        self.wireframeEnabled = 1

    def wireframeOff(self):
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
            self.mouse2cam.reparentTo(self.dataUnused)

    def enableMouse(self):
        """
        Reverse the effect of a previous call to disableMouse().
        useDrive() also implicitly enables the mouse.
        """
        if self.mouse2cam:
            self.mouse2cam.reparentTo(self.mouseInterface)

    def setMouseOnNode(self, newNode):
        if self.mouse2cam:
            self.mouse2cam.node().setNode(newNode)

    def changeMouseInterface(self, changeTo):
        """
        Switch mouse action
        """
        # Get rid of the prior interface:
        self.mouseInterface.reparentTo(self.dataUnused)
        # Update the mouseInterface to point to the drive
        self.mouseInterface = changeTo
        self.mouseInterfaceNode = self.mouseInterface.node()
        # Hookup the drive to the camera.
        self.mouseInterface.reparentTo(self.mouseWatcher)
        if self.mouse2cam:
            self.mouse2cam.reparentTo(self.mouseInterface)

    def useDrive(self):
        """
        Switch mouse action to drive mode
        """
        if self.drive:
            self.changeMouseInterface(self.drive)
            # Set the height to a good eyeheight
            self.mouseInterfaceNode.reset()
            self.mouseInterfaceNode.setZ(4.0)

    def useTrackball(self):
        """
        Switch mouse action to trackball mode
        """
        if self.trackball:
            self.changeMouseInterface(self.trackball)

    def oobe(self):
        """
        Enable a special "out-of-body experience" mouse-interface
        mode.  This can be used when a "god" camera is needed; it
        moves the camera node out from under its normal node and sets
        the world up in trackball state.  Button events are still sent
        to the normal mouse action node (e.g. the DriveInterface), and
        mouse events, if needed, may be sent to the normal node by
        holding down the Control key.

        This is different than useTrackball(), which simply changes
        the existing mouse action to a trackball interface.  In fact,
        OOBE mode doesn't care whether useDrive() or useTrackball() is
        in effect; it just temporarily layers a new trackball
        interface on top of whatever the basic interface is.  You can
        even switch between useDrive() and useTrackball() while OOBE
        mode is in effect.

        This is a toggle; the second time this function is called, it
        disables the mode.
        """
        # If oobeMode was never set, set it to false and create the
        # structures we need to implement OOBE.
        try:
            self.oobeMode
        except:
            self.oobeMode = 0

            self.oobeCamera = self.hidden.attachNewNode('oobeCamera')
            self.oobeCameraTrackball = self.oobeCamera.attachNewNode('oobeCameraTrackball')
            self.oobeLens = PerspectiveLens()
            self.oobeLens.setAspectRatio(self.getAspectRatio())
            self.oobeLens.setNearFar(0.1, 10000.0)
            self.oobeLens.setFov(52.0)

            self.oobeTrackball = self.dataUnused.attachNewNode(Trackball('oobeTrackball'), 1)
            self.oobe2cam = self.oobeTrackball.attachNewNode(Transform2SG('oobe2cam'))
            self.oobe2cam.node().setNode(self.oobeCameraTrackball.node())

            self.oobeVis = loader.loadModelOnce('models/misc/camera')
            if self.oobeVis:
                self.oobeVis.node().setFinal(1)
            self.oobeCullFrustum = None
            self.oobeCullFrustumVis = None

        if self.oobeMode:
            # Disable OOBE mode.
            if self.oobeCullFrustum != None:
                # First, disable OOBE cull mode.
                self.oobeCull()

            if self.oobeVis:
                self.oobeVis.reparentTo(self.hidden)

            # Restore the mouse interface node.
            #self.mouseInterface.reparentTo(self.mouseWatcher)
            self.oobeTrackball.reparentTo(self.dataUnused)

            self.cam.reparentTo(self.camera)
            self.camNode.setLens(self.camLens)
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

            # Move aside the current mouse interface node and put the
            # oobeTrackball in its place.
            #self.mouseInterface.reparentTo(self.dataUnused)
            self.oobeTrackball.reparentTo(self.mouseWatcher)

            # Set our initial OOB position to be just behind the camera.
            mat = Mat4.translateMat(0, -10, 3) * self.camera.getMat(cameraParent)
            mat.invertInPlace()
            self.oobeTrackball.node().setMat(mat)

            self.cam.reparentTo(self.oobeCameraTrackball)

            # Don't change the camera lens--keep it with the original lens.
            #self.camNode.setLens(self.oobeLens)

            if self.oobeVis:
                self.oobeVis.reparentTo(self.camera)
            self.oobeMode = 1

    def oobeCull(self):
        """
        While in OOBE mode (see above), cull the viewing frustum as if
        it were still attached to our original camera.  This allows us
        to visualize the effectiveness of our bounding volumes.
        """
        # First, make sure OOBE mode is enabled.
        try:
            if not self.oobeMode:
                self.oobe()
        except:
            self.oobe()

        if self.oobeCullFrustum == None:
            # Enable OOBE culling.
            pnode = LensNode('oobeCull')
            pnode.setLens(self.camLens)
            self.oobeCullFrustum = self.camera.attachNewNode(pnode)

            # Create a visible representation of the frustum.
            geom = self.camLens.makeGeometry()
            if geom != None:
                gn = GeomNode('frustum')
                gn.addGeom(geom)
                self.oobeCullFrustumVis = self.oobeVis.attachNewNode(gn)

            # Tell the camera to cull from here instead of its own
            # origin.
            self.camNode.setCullCenter(self.oobeCullFrustum)
        else:
            # Disable OOBE culling.

            self.camNode.setCullCenter(NodePath())
            self.oobeCullFrustum.removeNode()
            self.oobeCullFrustum = None

            if self.oobeCullFrustumVis != None:
                self.oobeCullFrustumVis.removeNode()
                self.oobeCullFrustumVis = None

    def showCameraFrustum(self):
        # Create a visible representation of the frustum.
        self.removeCameraFrustum()
        geom = self.camLens.makeGeometry()
        if geom != None:
            gn = GeomNode('frustum')
            gn.addGeom(geom)
            self.camFrustumVis = self.camera.attachNewNode(gn)

    def removeCameraFrustum(self):
        if self.camFrustumVis:
            self.camFrustumVis.removeNode()

    def screenshot(self, namePrefix = 'screenshot',
                   defaultFilename = 1, source = None):
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
        mark ('#') character. """
        
        if source == None:
            source = self.win

        if defaultFilename:
            filename = GraphicsOutput.makeScreenshotFilename(namePrefix)
        else:
            filename = Filename(namePrefix)

        if isinstance(source, Texture):
            if source.getZSize() > 1:
                saved = source.writePages(filename)
            else:
                saved = source.write(filename)
        else:
            saved = source.saveScreenshot(filename)

        # Announce to anybody that a screenshot has been taken
        messenger.send('screenshot', [filename])

        return saved

    def saveCubeMap(self, namePrefix = 'cube_map_#.png',
                    defaultFilename = 0, source = None,
                    camera = None, size = 128,
                    cameraMask = BitMask32.allOn()):

        """ Similar to screenshot(), this sets up a temporary cube map
        Texture which it uses to take a series of six snapshots of the
        current scene, one in each of the six cube map directions.
        This requires rendering a new frame.

        Unlike screenshot(), source may only be a GraphicsWindow,
        GraphicsBuffer, or DisplayRegion; it may not be a Texture.

        camera should be the node to which the cubemap cameras will be
        parented.  The default is the camera associated with source,
        if source is a DisplayRegion, or base.camera otherwise."""

        if source == None:
            source = base.win

        if camera == None:
            if hasattr(source, "getCamera"):
                camera = source.getCamera()
            if camera == None:
                camera = base.camera

        if hasattr(source, "getWindow"):
            source = source.getWindow()

        rig = NodePath(namePrefix)
        buffer = source.makeCubeMap(namePrefix, size, 1, rig, cameraMask)
        if buffer == None:
            raise StandardError, "Could not make cube map."

        # Set the near and far planes from the default lens.
        lens = rig.find('**/+Camera').node().getLens()
        lens.setNearFar(base.camLens.getNear(), base.camLens.getFar())

        # Now render a frame to fill up the texture.
        rig.reparentTo(camera)
        base.graphicsEngine.openWindows()
        base.graphicsEngine.renderFrame()

        tex = buffer.getTexture()
        saved = self.screenshot(namePrefix = namePrefix,
                                defaultFilename = defaultFilename,
                                source = tex)

        base.graphicsEngine.removeWindow(buffer)
        rig.removeNode()

        return saved

    def saveSphereMap(self, namePrefix = 'spheremap.png',
                      defaultFilename = 0, source = None,
                      camera = None, size = 256,
                      cameraMask = BitMask32.allOn(),
                      numVertices = 1000):

        """ This works much like saveCubeMap(), and uses the graphics
        API's hardware cube-mapping ability to get a 360-degree view
        of the world.  But then it converts the six cube map faces
        into a single fisheye texture, suitable for applying as a
        static environment map (sphere map).

        For eye-relative static environment maps, sphere maps are
        often preferable to cube maps because they require only a
        single texture and because they are supported on a broader
        range of hardware.  """

        if source == None:
            source = base.win

        if camera == None:
            if hasattr(source, "getCamera"):
                camera = source.getCamera()
            if camera == None:
                camera = base.camera

        if hasattr(source, "getWindow"):
            source = source.getWindow()

        # First, make an offscreen buffer to convert the cube map to a
        # sphere map.  We make it first so we can guarantee the
        # rendering order for the cube map.
        toSphere = source.makeTextureBuffer(namePrefix, size, size,
                                            Texture(), 1)

        # Now make the cube map buffer.
        rig = NodePath(namePrefix)
        buffer = toSphere.makeCubeMap(namePrefix, size, 0, rig, cameraMask)
        if buffer == None:
            base.graphicsEngine.removeWindow(toSphere)
            raise StandardError, "Could not make cube map."

        # Set the near and far planes from the default lens.
        lens = rig.find('**/+Camera').node().getLens()
        lens.setNearFar(base.camLens.getNear(), base.camLens.getFar())

        # Set up the scene to convert the cube map.  It's just a
        # simple scene, with only the FisheyeMaker object in it.
        dr = toSphere.makeDisplayRegion()
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
        base.graphicsEngine.openWindows()
        base.graphicsEngine.renderFrame()

        saved = self.screenshot(namePrefix = namePrefix,
                                defaultFilename = defaultFilename,
                                source = toSphere.getTexture())

        base.graphicsEngine.removeWindow(buffer)
        base.graphicsEngine.removeWindow(toSphere)
        rig.removeNode()

        return saved

    def movie(self, namePrefix = 'movie', duration = 1.0, fps = 30,
              format = 'png', sd = 4, source = None):
        """
        Spawn a task to capture a movie using the screenshot function.
        - namePrefix will be used to form output file names (can include
          path information (e.g. '/i/beta/frames/myMovie')
        - duration is the length of the movie in seconds
        - fps is the frame rate of the resulting movie
        - format specifies output file format (e.g. png, bmp)
        - sd specifies number of significant digits for frame count in the
          output file name (e.g. if sd = 4, movie_0001.png)
        - source is the Window, Buffer, DisplayRegion, or Texture from which
          to save the resulting images.  The default is the main window.
        """
        globalClock.setMode(ClockObject.MNonRealTime)
        globalClock.setDt(1.0/float(fps))
        t = taskMgr.add(self._movieTask, namePrefix + '_task')
        t.frameIndex = 1
        t.numFrames = int(duration * fps)
        t.source = source
        t.outputString = namePrefix + '_%0' + `sd` + 'd.' + format
        t.uponDeath = lambda state: globalClock.setMode(ClockObject.MNormal)

    def _movieTask(self, state):
        frameName = state.outputString % state.frameIndex
        self.notify.info("Capturing frame: " + frameName)
        self.screenshot(namePrefix = frameName, defaultFilename = 0,
                        source = state.source)
                
        state.frameIndex += 1
        if state.frameIndex > state.numFrames:
            return Task.done
        else:
            return Task.cont

    def __windowEvent(self, win):
        if win == self.win:
            properties = win.getProperties()
            self.notify.info("Got window event: %s" % (repr(properties)))
            if not properties.getOpen():
                # If the user closes the main window, we should exit.
                self.notify.info("User closed main window.")
                self.userExit()

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

    def userExit(self):
        # The user has requested we exit the program.  Deal with this.
        if self.exitFunc:
            self.exitFunc()
        self.notify.info("Exiting ShowBase.")
        self.finalizeExit()

    def finalizeExit(self):
        sys.exit()

    def startTk(self, fWantTk = 1):
        self.wantTk = fWantTk
        if self.wantTk:
            import TkGlobal
            taskMgr.remove('tkLoop')
            TkGlobal.spawnTkLoop()

    def startDirect(self, fWantDirect = 1, fWantTk = 1):
        self.startTk(fWantTk)
        self.wantDirect = fWantDirect
        if self.wantDirect:
            from direct.directtools import DirectSession
            direct.enable()
        else:
            __builtins__["direct"] = self.direct = None

    def run(self):
        self.taskMgr.run()


# A class to encapsulate information necessary for multiwindow support.
class WindowControls:
    def __init__(self, win, cam=None, cam2d=None, mouseWatcher=None, mouseKeyboard=None, closeCmd=lambda : 0):
        self.win = win
        self.camera = cam
        self.camera2d = cam2d
        self.mouseWatcher = mouseWatcher
        self.mouseKeyboard = mouseKeyboard
        self.closeCommand = closeCmd

    def __str__(self):
        s = "window = " + str(self.win) + "\n"
        s += "camera = " + str(self.camera) + "\n"
        s += "camera2d = " + str(self.camera2d) + "\n"
        s += "mouseWatcher = " + str(self.mouseWatcher) + "\n"
        s += "mouseAndKeyboard = " + str(self.mouseKeyboard) + "\n"
        return s

    
