
# This module redefines the builtin import function with one
# that prints out every import it does in a hierarchical form
# Annoying and very noisy, but sometimes useful
# import VerboseImport

from PandaModules import *
from DirectNotifyGlobal import *
from MessengerGlobal import *
from TaskManagerGlobal import *
from EventManagerGlobal import *
from PythonUtil import *
from ParticleManagerGlobal import *
from PhysicsManagerGlobal import *

import Task
import EventManager
import math
import sys
import Loader
import time
import FSM
import State
import __builtin__

__builtin__.FADE_SORT_INDEX = 1000
__builtin__.NO_FADE_SORT_INDEX = 2000

globalClock = ClockObject.getGlobalClock()
class ShowBase:

    notify = directNotify.newCategory("ShowBase")

    def __init__(self):

        # Get the dconfig object
        self.config = ConfigConfigureGetConfigConfigShowbase

        # Store dconfig variables
        self.wantTk = self.config.GetBool('want-tk', 0)
        self.sfxActive = self.config.GetBool('audio-sfx-active', 1)
        self.musicActive = self.config.GetBool('audio-music-active', 1)
        self.wantFog = self.config.GetBool('want-fog', 1)

        self.screenshotExtension = self.config.GetString('screenshot-extension', 'jpg')
        self.musicManager = None
        self.musicManagerIsValid = None
        self.sfxManager = None
        self.sfxManagerIsValid = None

        self.wantDIRECT = self.config.GetBool('want-directtools', 0)
        self.wantStats = self.config.GetBool('want-stats', 0)

        taskMgr.taskTimerVerbose = self.config.GetBool('task-timer-verbose', 0)
        taskMgr.extendedExceptions = self.config.GetBool('extended-exceptions', 0)
        
        taskMgr.pStatsTasks = self.config.GetBool('pstats-tasks', 0)

        # Set up the TaskManager to reset the PStats clock back
        # whenever we resume from a pause.  This callback function is
        # a little hacky, but we can't call it directly from within
        # the TaskManager because he doesn't know about PStats (and
        # has to run before libpanda is even loaded).
        taskMgr.resumeFunc = PStatClient.resumeAfterPause

        fsmRedefine = self.config.GetBool('fsm-redefine', 0)
        State.FsmRedefine = fsmRedefine

        self.hidden = NodePath('hidden')

        # We need a graphics engine to manage the actual rendering.
        self.graphicsEngine = GraphicsEngine()

        self.setupRender()
        self.setupRender2d()
        self.setupDataGraph()        

        # This is a placeholder for a CollisionTraverser.  If someone
        # stores a CollisionTraverser pointer here, we'll traverse it
        # in the igloop task.
        self.cTrav = 0
        # Ditto for an AppTraverser.
        self.appTrav = 0

        # This is the DataGraph traverser, which we might as well
        # create now.
        self.dgTrav = DataGraphTraverser()

        # base.win is the main, or only window; base.winList is a list of
        # *all* windows.  Similarly with base.pipeList and base.camList.
        self.win = None
        self.winList = []
        self.pipe = None
        self.pipeList = []
        self.cam = None
        self.camList = []
        self.camNode = None
        self.camLens = None

        # base.camera is a little different; rather than referring to
        # base.cameraList[0], it is instead the parent node of all
        # cameras in base.cameraList.  That way, multiple cameras can
        # easily be dragged around by moving the one node.
        self.camera = self.render.attachNewNode('camera')
        self.cameraList = []
        self.groupList = []
        self.camera2d = self.render2d.attachNewNode('camera2d')

        # Now that we've set up the window structures, assign an exitfunc.
        self.oldexitfunc = getattr(sys, 'exitfunc', None)
        sys.exitfunc = self.exitfunc

        # Open the default rendering window.
        if self.config.GetBool('open-default-window', 1):
            self.openWindow()
            self.setupMouse(self.win)
            self.makeCamera2d(self.win, -1, 1, -1, 1)

        self.loader = Loader.Loader(self)
        self.eventMgr = eventMgr
        self.messenger = messenger
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

        self.createAudioManager()
        self.createStats()

        self.AppHasAudioFocus = 1

        __builtin__.base = self
        __builtin__.render2d = self.render2d
        __builtin__.aspect2d = self.aspect2d
        __builtin__.render = self.render
        __builtin__.hidden = self.hidden
        __builtin__.camera = self.camera
        __builtin__.loader = self.loader
        __builtin__.taskMgr = self.taskMgr
        __builtin__.eventMgr = self.eventMgr
        __builtin__.messenger = self.messenger
        __builtin__.config = self.config
        __builtin__.run = self.run
        __builtin__.ostream = Notify.out()
        __builtin__.directNotify = directNotify

        # Transition effects (fade, iris, etc)
        import Transitions
        self.transitions = Transitions.Transitions(self.loader)

        # Tk
        if self.wantTk:
            import TkGlobal
        if self.wantDIRECT:
            import DirectSession
            direct.enable()
        else:
            __builtin__.direct = self.direct = None

        self.restart()

    def exitfunc(self):
        """exitfunc(self)

        This should be assigned to sys.exitfunc to be called just
        before Python shutdown.  It guarantees that the Panda window
        is closed cleanly, so that we free system resources, restore
        the desktop and keyboard functionality, etc.
        """
        for win in self.winList:
            win.closeWindow()
        del self.win
        del self.winList
        del self.pipe

        if self.oldexitfunc:
            self.oldexitfunc()

    def openWindow(self):
        """openWindow(self)

        Invokes ChanConfig to create a window and adds it to the list
        of windows that are to be updated every frame.

        """

        if self.pipe == None:
            self.pipe = makeGraphicsPipe()
            self.pipeList.append(self.pipe)

        chanConfig = qpmakeGraphicsWindow(self.pipe, self.render)
        win = chanConfig.getWin()

        if self.win == None:
            self.win = win

        self.winList.append(win)
        self.graphicsEngine.addWindow(win)

        self.getCameras(chanConfig)


    def setupRender(self):
        """setupRender(self)

        Creates the render scene graph, the primary scene graph for
        rendering 3-d geometry.
        """
        self.render = NodePath('render')

    def setupRender2d(self):
        """setupRender2d(self)

        Creates the render2d scene graph, the primary scene graph for
        2-d objects and gui elements that are superimposed over the
        3-d geometry in the window.
        """

        self.render2d = NodePath('render2d')

        # Set up some overrides to turn off certain properties which
        # we probably won't need for 2-d objects.

        # It's particularly important to turn off the depth test,
        # since we'll be keeping the same depth buffer already filled
        # by the previously-drawn 3-d scene--we don't want to pay for
        # a clear operation, but we also don't want to collide with
        # that depth buffer.
        dt = DepthTestAttrib.make(DepthTestAttrib.MNone)
        dw = DepthWriteAttrib.make(DepthWriteAttrib.MOff)
        #lt = LightTransition.allOff()
        self.render2d.node().setAttrib(dt, 1)
        self.render2d.node().setAttrib(dw, 1)
        #self.render2d.node().setAttrib(lt, 1)

        self.render2d.setMaterialOff(1)
        self.render2d.setTwoSided(1, 1)
        
        # The normal 2-d layer has an aspect ratio that matches the
        # window, but its coordinate system is square.  This means
        # anything we parent to render2d gets stretched.  For things
        # where that makes a difference, we set up aspect2d, which
        # scales things back to the right aspect ratio.

        # For now, we assume that the window will have an aspect ratio
        # matching that of a traditional PC screen (w / h) = (4 / 3)
        self.aspectRatio = self.config.GetFloat('aspect-ratio', (4.0 / 3.0))

        #self.aspect2d = self.render2d.attachNewNode(PGTop("aspect2d"))
        self.aspect2d = self.render2d.attachNewNode("aspect2d")
        self.aspect2d.setScale(1.0 / self.aspectRatio, 1.0, 1.0)

        # It's important to know the bounds of the aspect2d screen.
        self.a2dTop = 1.0
        self.a2dBottom = -1.0
        self.a2dLeft = -self.aspectRatio
        self.a2dRight = self.aspectRatio

    def makeCamera2d(self, win, left, right, bottom, top):
        """makeCamera2d(self)

        Makes a new camera2d associated with the indicated window, and
        assigns it to render the indicated subrectangle of render2d.
        """

        # First, we need to make a new layer on the window.
        chan = win.getChannel(0)
        layer = chan.makeLayer()

        # And make a display region to cover the whole layer.
        dr = layer.makeDisplayRegion()

        # Now make a new Camera node.
        cam2dNode = Camera('cam2d')
        lens = OrthographicLens()
        lens.setFilmSize(right - left, top - bottom)
        lens.setFilmOffset((right + left) / 2.0, (top + bottom) / 2.0)
        lens.setNearFar(-1000, 1000)
        cam2dNode.setLens(lens)
        cam2dNode.setScene(self.render2d)
        camera2d = self.camera2d.attachNewNode(cam2dNode)
        dr.setCamera(camera2d)

        return camera2d


    def setupDataGraph(self):
        """setupDataGraph(self)

        Creates the data graph and populates it with the basic input
        devices.
        """
        
        self.dataRoot = NodePath('dataRoot')
        # Cache the node so we do not ask for it every frame
        self.dataRootNode = self.dataRoot.node()
        self.dataUnused = NodePath('dataUnused')


    def setupMouse(self, win):
        """setupMouse(self, win)

        Creates the structures necessary to monitor the mouse input,
        using the indicated window.  This should only be called once
        per application.
        """

        # We create both a MouseAndKeyboard object and a MouseWatcher object
        # for the window.  The MouseAndKeyboard generates mouse events and
        # mouse button/keyboard events; the MouseWatcher passes them through
        # unchanged when the mouse is not over a 2-d button, and passes
        # nothing through when the mouse *is* over a 2-d button.  Therefore,
        # objects that don't want to get events when the mouse is over a
        # button, like the driveInterface, should be parented to
        # mouseWatcher, while objects that want events in all cases, like the
        # chat interface, should be parented to mak.
        self.mak = self.dataRoot.attachNewNode(MouseAndKeyboard(win, 0, 'mak'))
        self.mouseWatcherNode = MouseWatcher('mouseWatcher')
        self.mouseWatcher = self.mak.attachNewNode(self.mouseWatcherNode)
        mb = self.mouseWatcherNode.getModifierButtons()
        mb.addButton(KeyboardButton.shift())
        mb.addButton(KeyboardButton.control())
        mb.addButton(KeyboardButton.alt())
        self.mouseWatcherNode.setModifierButtons(mb)

        # Now we have the main trackball & drive interfaces.
        # useTrackball() and useDrive() switch these in and out; only
        # one is in use at a given time.
        self.trackball = self.dataUnused.attachNewNode(Trackball('trackball'))
        self.drive = self.dataUnused.attachNewNode(DriveInterface('drive'))
        self.mouse2cam = self.dataUnused.attachNewNode(Transform2SG('mouse2cam'))
        self.mouse2cam.node().setNode(self.camera.node())

        # The default is trackball mode, which is more convenient for
        # ad-hoc development in Python using ShowBase.  Applications
        # can expclitly call base.useDrive() if they prefer a drive
        # interface.
        self.useTrackball()

        # A ButtonThrower to generate events from the mouse and
        # keyboard buttons as they are pressed.
        self.buttonThrower = self.mouseWatcher.attachNewNode(ButtonThrower('buttons'))

        # Specialize the events based on whether the modifier keys are
        # being held down.
        mods = ModifierButtons()
        mods.addButton(KeyboardButton.shift())
        mods.addButton(KeyboardButton.control())
        mods.addButton(KeyboardButton.alt())
        self.buttonThrower.node().setModifierButtons(mods)

        # Tell the gui system about our new mouse watcher.
        #self.aspect2d.node().setMouseWatcher(self.mouseWatcherNode)
        #self.mouseWatcherNode.addRegion(PGMouseWatcherBackground())

    def enableSoftwareMousePointer(self):
        """enableSoftwareMousePointer(self)

        Creates some geometry and parents it to render2d to show
        the currently-known mouse position.  Useful if the mouse
        pointer is invisible for some reason.
        """
        mouseViz = render2d.attachNewNode('mouseViz')
        lilsmiley = loader.loadModel('lilsmiley')
        lilsmiley.reparentTo(mouseViz)
        # Scale the smiley face to 32x32 pixels.
        lilsmiley.setScale(32.0 / self.win.getHeight() / self.aspectRatio, 1.0, 32.0 / self.win.getHeight())
        #self.mouseWatcherNode.setGeometry(mouseViz)
        

    def getCameras(self, chanConfig):
        """getCameras(self, chanConfig)

        Extracts the camera(s) out of the ChanConfig record, parents
        them all to base.camera, and adds them to base.cameraList.
        """

        # cameraList is a list of camera group nodes.  There may
        # be more than one display region/camera node beneath each
        # one.
        for i in range(chanConfig.getNumGroups()):
            camera = self.camera.attachNewNode(chanConfig.getGroupNode(i))
            cam = camera.find('**/+Camera')
            lens = cam.node().getLens()

            # Enforce our expected aspect ratio, overriding whatever
            # nonsense ChanConfig put in there.
            lens.setAspectRatio(self.aspectRatio)
            
            self.cameraList.append(camera)
            self.camList.append(cam)
            
        # this is how we know which display region cameras belong to which
        # camera group.  display region i belongs to group self.groupList[i]
        for i in range(chanConfig.getNumDrs()):
            self.groupList.append(chanConfig.getGroupMembership(i))

        # Set the default camera
        if self.cam == None:
            self.cam = self.camList[0]

            # If you need to get a handle to the camera node itself,
            # use self.camNode.
            self.camNode = self.cam.node()

            # If you need to adjust camera parameters, like fov or
            # near/far clipping planes, use self.camLens.
            self.camLens = self.camNode.getLens()


    def getAlt(self):
        return base.mouseWatcherNode.getModifierButtons().isDown(
            KeyboardButton.alt())

    def getShift(self):
        return base.mouseWatcherNode.getModifierButtons().isDown(
            KeyboardButton.shift())

    def getControl(self):
        return base.mouseWatcherNode.getModifierButtons().isDown(
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
        """updateManagers(self)"""
        dt = min(globalClock.getDt(), 0.1)
        if (self.particleMgrEnabled == 1):
            self.particleMgr.doParticles(dt)
        if (self.physicsMgrEnabled == 1):
            self.physicsMgr.doPhysics(dt)
        return Task.cont

    def createStats(self):
        # You must specify a pstats-host in your configrc
        # The default is localhost
        if self.wantStats:
            PStatClient.connect()

    def createAudioManager(self):
        self.sfxManager = AudioManager.createAudioManager()
        self.sfxManagerIsValid=self.sfxManager!=None \
                and self.sfxManager.isValid()
        if self.sfxManagerIsValid:
            self.sfxManager.setActive(self.sfxActive)

        self.musicManager = AudioManager.createAudioManager()
        self.musicManagerIsValid=self.musicManager!=None \
                and self.musicManager.isValid()
        if self.musicManagerIsValid:
            self.musicManager.setActive(self.musicActive)
            # Turn down the music globally
            # Eventually we may want to control this in the options page
            self.musicManager.setVolume(0.7)

    def loadSfx(self, name):
        if (name):
            sound=self.sfxManager.getSound(name)
            if sound == None:
                self.notify.warning("Could not load sound file %s." % name)
            return sound

    def loadMusic(self, name):
        if (name):
            sound=self.musicManager.getSound(name)
            if sound == None:
                self.notify.warning("Could not load music file %s." % name)
            return sound

    def playSfx(self, sfx, looping = 0, interupt = 1, volume = None,
            time = 0.):
        if sfx:
            if volume != None:
                sfx.setVolume(volume)
            if interupt or (sfx.status() != AudioSound.PLAYING):
                sfx.setTime(time)
                sfx.setLoop(looping)
                sfx.play()

    def playMusic(self, music, looping = 0, interupt = 1, volume = None,
            time = 0.0):
        if music:
            if volume != None:
                music.setVolume(volume)
            if interupt or (music.status() != AudioSound.PLAYING):
                music.setTime(time)
                music.setLoop(looping)
                music.play()

    def dataloop(self, state):
        # traverse the data graph.  This reads all the control
        # inputs (from the mouse and keyboard, for instance) and also
        # directly acts upon them (for instance, to move the avatar).
        self.dgTrav.traverse(self.dataRootNode)
        return Task.cont

    def igloop(self, state):
        # run the collision traversal if we have a
        # CollisionTraverser set.
        if self.cTrav:
            self.cTrav.traverse(self.render)
        if self.appTrav:
            self.appTrav.traverse(self.render)
            
        # Finally, render the frame.
        self.graphicsEngine.renderFrame()

        # Lerp stuff needs this event, and it must be generated in
        # C++, not in Python.
        throwNewFrame()
        return Task.cont

    def restart(self):
        self.shutdown()
        # give the igloop task a reasonably "late" priority,
        # so that it will get run after most tasks
        self.taskMgr.add(self.igloop, 'igloop', priority = 50)
        # give the dataloop task a reasonably "early" priority,
        # so that it will get run before most tasks
        self.taskMgr.add(self.dataloop, 'dataloop', priority = -50)
        self.eventMgr.restart()

    def shutdown(self):
        self.taskMgr.remove('igloop')
        self.taskMgr.remove('dataloop')
        self.eventMgr.shutdown()

    def toggleBackface(self):
        return toggleBackface(self.render.arc())

    def backfaceCullingOn(self):
        if self.toggleBackface():
            self.toggleBackface()

    def backfaceCullingOff(self):
        if not self.toggleBackface():
            self.toggleBackface()

    def toggleTexture(self):
        return toggleTexture(self.render.arc())

    def textureOn(self):
        if not self.toggleTexture():
            self.toggleTexture()

    def textureOff(self):
        if self.toggleTexture():
            self.toggleTexture()

    def toggleWireframe(self):
        return toggleWireframe(self.render.arc())

    def wireframeOn(self):
        if not self.toggleWireframe():
            self.toggleWireframe()

    def wireframeOff(self):
        if self.toggleWireframe():
            self.toggleWireframe()

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
        self.mouse2cam.reparentTo(self.dataUnused)

    def enableMouse(self):
        """
        Reverse the effect of a previous call to disableMouse().
        useDrive() also implicitly enables the mouse.
        """
        self.mouse2cam.reparentTo(self.mouseInterface)

    def setMouseOnArc(self, newArc):
        self.mouse2cam.node().setArc(newArc)

    def useDrive(self):
        """
        Switch mouse action to drive mode
        """
        # Get rid of the trackball
        self.trackball.reparentTo(self.dataUnused)
        # Update the mouseInterface to point to the drive
        self.mouseInterface = self.drive
        self.mouseInterfaceNode = self.mouseInterface.node()
        self.drive.node().reset()
        # Hookup the drive to the camera.
        self.drive.reparentTo(self.mouseWatcher)
        self.mouse2cam.reparentTo(self.drive)
        # Set the height to a good eyeheight
        self.drive.node().setZ(4.0)

    def useTrackball(self):
        """
        Switch mouse action to trackball mode
        """
        # Get rid of the drive
        self.drive.reparentTo(self.dataUnused)
        # Update the mouseInterface to point to the trackball
        self.mouseInterface = self.trackball
        self.mouseInterfaceNode = self.mouseInterface.node()
        # Hookup the trackball to the camera.
        self.trackball.reparentTo(self.mouseWatcher)
        self.mouse2cam.reparentTo(self.trackball)

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
            self.oobeLens.setAspectRatio(self.aspectRatio)
            self.oobeLens.setNearFar(0.1, 10000.0)
            self.oobeLens.setFov(52.0)
            self.oobeControl = DataValve.Control()
            self.mouseValve.node().setControl(1, self.oobeControl)
            self.oobeTrackball = self.mouseValve.attachNewNode(Trackball('oobeTrackball'), 1)
            self.oobe2cam = self.oobeTrackball.attachNewNode(Transform2SG('oobe2cam'))
            self.oobe2cam.node().setArc(self.oobeCameraTrackball.arc())

            self.oobeButtonEventsType = TypeRegistry.ptr().findType('ButtonEvents_ButtonEventDataTransition')

            self.oobeVis = loader.loadModelOnce('models/misc/camera')
            if self.oobeVis:
                self.oobeVis.arc().setFinal(1)
            self.oobeCullFrustum = None
            self.oobeCullFrustumVis = None

            # Make sure the MouseValve is monitoring the Control key.
            mods = ModifierButtons(self.mouseValve.node().getModifierButtons())
            mods.addButton(KeyboardButton.control())
            self.mouseValve.node().setModifierButtons(mods)

        if self.oobeMode:
            # Disable OOBE mode.

            if self.oobeCullFrustum != None:
                # First, disable OOBE cull mode.
                self.oobeCull()
            
            self.oobeControl.setOff()
            self.mouseControl.setOn()
            if self.oobeVis:
                self.oobeVis.reparentTo(self.hidden)
            self.cam.reparentTo(self.camera)
            self.camNode.setLens(self.camLens)
            self.oobeCamera.reparentTo(self.hidden)
            self.oobeMode = 0            
        else:
            # Enable OOBE mode.
            mods = ModifierButtons(self.mouseValve.node().getModifierButtons())

            # We're in OOBE control mode without the control key.
            mods.allButtonsUp()
            self.oobeControl.setButtons(mods)

            # We're in traditional control mode with the control key.
            mods.buttonDown(KeyboardButton.control())
            self.mouseControl.setButtons(mods)

            # However, keyboard buttons always make it through to the
            # traditional controller, regardless of the control key.
            self.mouseValve.node().setFineControl(0, self.oobeButtonEventsType, self.onControl)

            # Make oobeCamera be a sibling of wherever camera is now.
            cameraParent = self.camera.getParent()
            self.oobeCamera.reparentTo(cameraParent)
            self.oobeCamera.clearMat()

            # Set our initial OOB position to be just behind the camera.
            mat = Mat4.translateMat(0, -10, 3) * self.camera.getMat(cameraParent)
            mat.invertInPlace()
            self.oobeTrackball.node().setMat(mat)

            self.cam.reparentTo(self.oobeCameraTrackball)
            self.camNode.setLens(self.oobeLens)
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

            # Assign each DisplayRegion shared by the camera to use
            # this cull frustum.
            numDrs = self.camNode.getNumDrs()
            for d in range(0, numDrs):
                dr = self.camNode.getDr(d)
                dr.setCullFrustum(pnode)
        else:
            # Disable OOBE culling.

            # Assign each DisplayRegion shared by the camera to use
            # the default cull frustum, the camera itself.
            numDrs = self.camNode.getNumDrs()
            for d in range(0, numDrs):
                dr = self.camNode.getDr(d)
                dr.setCullFrustum(self.camNode)

            self.oobeCullFrustum.removeNode()
            self.oobeCullFrustum = None

            if self.oobeCullFrustumVis != None:
                self.oobeCullFrustumVis.removeNode()
                self.oobeCullFrustumVis = None

    def screenshot(self, namePrefix='screenshot'):
        # Get the current date and time to uniquify the image (down to the second)
        date = time.ctime(time.time())
        # Get the current frame count to uniqify it even more
        frameCount = globalClock.getFrameCount()
        # Replace spaces with dashes because unix does not like spaces in the filename
        date = date.replace(' ', '-')
        date = date.replace(':', '-')
        imageName = ('%s-%s-%d.%s' % (namePrefix, date, frameCount, self.screenshotExtension))
        self.notify.info("Taking screenshot: " + imageName)
        takeSnapshot(self.win, imageName)

    def movie(self, namePrefix = 'movie', duration = 1.0, fps = 30,
              format = 'rgb', sd = 4):
        """
        movie(namePrefix = 'movie', duration=1.0, fps=30, format='rgb', sd=4)

        Spawn a task to capture a movie using the takeSnapshot function.
        - namePrefix will be used to form output file names (can include
          path information (e.g. 'I:/beta/frames/myMovie')
        - duration is the length of the movie in seconds
        - fps is the frame rate of the resulting movie
        - format specifies output file format (e.g. rgb, bmp)
        - sd specifies number of significant digits for frame count in the
          output file name (e.g. if sd = 4, movie_0001.rgb)
        """
        globalClock.setMode(ClockObject.MNonRealTime)
        globalClock.setDt(1.0/float(fps))
        t = taskMgr.add(self._movieTask, namePrefix + '_task')
        t.endT = globalClock.getFrameTime() + duration
        t.frameIndex = 1
        t.outputString = namePrefix + '_%0' + `sd` + 'd.' + format
        t.uponDeath = lambda state: globalClock.setMode(ClockObject.MNormal)

    def _movieTask(self, state):
        currT = globalClock.getFrameTime()
        if currT >= state.endT:
            return Task.done
        else:
            frameName = state.outputString % state.frameIndex
            self.notify.info("Capturing frame: " + frameName)
            takeSnapshot(self.win, frameName )
            state.frameIndex += 1
            return Task.cont

    # these are meant to be called in response to a user request
    def EnableMusic(self, bEnableMusic):
        # dont setActive(1) if no audiofocus
        if self.AppHasAudioFocus and self.musicManagerIsValid:
            self.musicManager.setActive(bEnableMusic)
        self.musicActive = bEnableMusic
        if bEnableMusic:
            self.notify.debug("Enabling music")
        else:
            self.notify.debug("Disabling music")

    def EnableSoundEffects(self, bEnableSoundEffects):
        # dont setActive(1) if no audiofocus
        if self.AppHasAudioFocus and self.sfxManagerIsValid:
            self.sfxManager.setActive(bEnableSoundEffects)
        self.sfxActive=bEnableSoundEffects
        if bEnableSoundEffects:
            self.notify.debug("Enabling sound effects")
        else:
            self.notify.debug("Disabling sound effects")

    # these are meant to be called by the sw when app loses audio focus (switched out)
    def DisableAudio(self):
        self.AppHasAudioFocus = 0
        if self.sfxManagerIsValid:
            self.sfxManager.setActive(0)
        if self.musicManagerIsValid:
            self.musicManager.setActive(0)
        self.notify.debug("Disabling audio")

    def EnableAudio(self):
        self.AppHasAudioFocus = 1
        if self.sfxManagerIsValid:
            self.sfxManager.setActive(self.sfxActive)
        if self.musicManagerIsValid:
            self.musicManager.setActive(self.musicActive)
        self.notify.debug("Enabling audio")

    def run(self):
        self.taskMgr.run()

