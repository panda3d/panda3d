
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
from IntervalManager import ivalMgr

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

class ShowBase:

    notify = directNotify.newCategory("ShowBase")

    def __init__(self):

        # Get the dconfig object
        self.config = ConfigConfigureGetConfigConfigShowbase

        if self.config.GetBool('use-vfs', 1):
            vfs = VirtualFileSystem.getGlobalPtr()
        else:
            vfs = None

        # Store dconfig variables
        self.wantTk = self.config.GetBool('want-tk', 0)
        self.sfxActive = self.config.GetBool('audio-sfx-active', 1)
        self.musicActive = self.config.GetBool('audio-music-active', 1)
        self.wantFog = self.config.GetBool('want-fog', 1)

        self.screenshotExtension = self.config.GetString('screenshot-extension', 'jpg')
        self.musicManager = None
        self.musicManagerIsValid = None
        self.sfxManagerList = []
        self.sfxManagerIsValidList = []

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
        self.camera = None
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

        self.createBaseAudioManagers()
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
        __builtin__.globalClock = ClockObject.getGlobalClock()
        __builtin__.vfs = vfs

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
        try:
            # Temporary try .. except for new window code
            for win in self.winList:
                win.closeWindow()
        except:
            pass
        del self.win
        del self.winList
        del self.pipe

        try:
            direct.panel.destroy()
        except StandardError:
            pass

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

        chanConfig = makeGraphicsWindow(self.graphicsEngine, self.pipe, self.render)
        win = chanConfig.getWin()

        if self.win == None:
            self.win = win

        self.winList.append(win)
        try:
            # temporary try..except to support new window code
            self.graphicsEngine.addWindow(win)
        except:
            pass

        self.getCameras(chanConfig)


    def setupRender(self):
        """setupRender(self)

        Creates the render scene graph, the primary scene graph for
        rendering 3-d geometry.
        """
        self.render = NodePath('render')
        self.render.setTwoSided(0)
        self.backfaceCullingEnabled = 1
        self.textureEnabled = 1
        self.wireframeEnabled = 0


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

        self.aspect2d = self.render2d.attachNewNode(PGTop("aspect2d"))
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
        lens.setFilmOffset((right + left) * 0.5, (top + bottom) * 0.5)
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
        # can explicitly call base.useDrive() if they prefer a drive
        # interface.
        self.mouseInterface = self.trackball
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
        self.aspect2d.node().setMouseWatcher(self.mouseWatcherNode)
        self.mouseWatcherNode.addRegion(PGMouseWatcherBackground())

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
        """
        getCameras(self, chanConfig)
        Extracts the camera(s) out of the ChanConfig record, parents
        them all to base.camera, and adds them to base.cameraList.
        """

        # cameraList is a list of camera group nodes.  There may
        # be more than one display region/camera node beneath each
        # one.
        for i in range(chanConfig.getNumGroups()):
            camera = self.render.attachNewNode(chanConfig.getGroupNode(i))
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

        # Set the default camera and cam
        if self.camera == None:
            self.camera = self.cameraList[0]
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

    def addSfxManager(self, extraSfxManager):
        # keep a list of sfx manager objects to apply settings to, since there may be others
        # in addition to the one we create here
        self.sfxManagerList.append(extraSfxManager)
        newSfxManagerIsValid = (extraSfxManager!=None) and extraSfxManager.isValid()
        self.sfxManagerIsValidList.append(newSfxManagerIsValid)
        if newSfxManagerIsValid:
            extraSfxManager.setActive(self.sfxActive)

    def createBaseAudioManagers(self):
        sfxManager = AudioManager.createAudioManager()
        self.addSfxManager(sfxManager)

        self.musicManager = AudioManager.createAudioManager()
        self.musicManagerIsValid=self.musicManager!=None \
                and self.musicManager.isValid()
        if self.musicManagerIsValid:
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

    def loadSfx(self, name):
        # showbase's sfxManager should always be at front of list
        if(not self.sfxManagerIsValidList[0]):
            # dont print a warning if mgr invalid, since they should already know
            return None
        sound = None
        if (name):
            sound=self.sfxManagerList[0].getSound(name)
        if sound == None:
            self.notify.warning("Could not load sound file %s." % name)
        return sound


    def loadMusic(self, name):
        if(not self.musicManagerIsValid):
            return None
        sound = None
        if (name):
            sound=self.musicManager.getSound(name)
        if sound == None:
            self.notify.warning("Could not load music file %s." % name)
        return sound

    def playSfx(self, sfx, looping = 0, interrupt = 1, volume = None, time = 0.0):
        if sfx:
            if volume != None:
                sfx.setVolume(volume)

            # dont start over if it's already playing, unless "interrupt" was specified
            if interrupt or (sfx.status() != AudioSound.PLAYING):
                sfx.setTime(time)
                sfx.setLoop(looping)
                sfx.play()

    def playMusic(self, music, looping = 0, interrupt = 1, volume = None, time = 0.0):
        if music:
            if volume != None:
                music.setVolume(volume)

            # if interrupt was set to 0, start over even if it's already playing
            if interrupt or (music.status() != AudioSound.PLAYING):
                music.setTime(time)
                music.setLoop(looping)
                music.play()

    def dataloop(self, state):
        # traverse the data graph.  This reads all the control
        # inputs (from the mouse and keyboard, for instance) and also
        # directly acts upon them (for instance, to move the avatar).
        self.dgTrav.traverse(self.dataRootNode)
        return Task.cont

    def ivalloop(self, state):
        # Execute all intervals in the global ivalMgr.
        ivalMgr.step()
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
        # spawn the ivalloop with a later priority, so that it will
        # run after most tasks, but before igloop.
        self.taskMgr.add(self.ivalloop, 'ivalloop', priority = 10)
        self.eventMgr.restart()

    def shutdown(self):
        self.taskMgr.remove('igloop')
        self.taskMgr.remove('dataloop')
        self.taskMgr.remove('ivalloop')
        self.eventMgr.shutdown()

    def getBackgroundColor(self):
        """ Returns the current window background color.  This assumes
        the window is set up to clear the color each frame (this is
        the normal setting). """
        return VBase4(self.win.getClearColor())

    def setBackgroundColor(self, *args):
        """ Sets the window background color to the indicated value.
        This assumes the window is set up to clear the color each
        frame (this is the normal setting).

        The color may be either a VBase3 or a VBase4, or a 3-component
        tuple, or the individual r, g, b parameters.
        """

        numArgs = len(args)
        if numArgs == 3 or numArgs == 4:
            color = VBase4(args[0], args[1], args[2], 1.0)
        elif numArgs == 1:
            arg = args[0]
            color = VBase4(arg[0], arg[1], arg[2], 1.0)
        else:
            raise TypeError, ('Invalid number of arguments: %d, expected 1, 3, or 4.' % numArgs)
            
        self.win.setClearColor(color)
                
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
        self.render.setRenderModeWireframe(100);
        self.render.setTwoSided(1);
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
        self.mouse2cam.reparentTo(self.dataUnused)

    def enableMouse(self):
        """
        Reverse the effect of a previous call to disableMouse().
        useDrive() also implicitly enables the mouse.
        """
        self.mouse2cam.reparentTo(self.mouseInterface)

    def setMouseOnNode(self, newNode):
        self.mouse2cam.node().setNode(newNode)

    def useDrive(self):
        """
        Switch mouse action to drive mode
        """
        # Get rid of the trackball
        self.mouseInterface.reparentTo(self.dataUnused)
        # Update the mouseInterface to point to the drive
        self.mouseInterface = self.drive
        self.mouseInterfaceNode = self.mouseInterface.node()
        # Hookup the drive to the camera.
        self.mouseInterface.reparentTo(self.mouseWatcher)
        self.mouse2cam.reparentTo(self.mouseInterface)
        # Set the height to a good eyeheight
        self.mouseInterfaceNode.reset()
        self.mouseInterfaceNode.setZ(4.0)

    def useTrackball(self):
        """
        Switch mouse action to trackball mode
        """
        # Get rid of the drive
        self.mouseInterface.reparentTo(self.dataUnused)
        # Update the mouseInterface to point to the trackball
        self.mouseInterface = self.trackball
        self.mouseInterfaceNode = self.mouseInterface.node()
        # Hookup the trackball to the camera.
        self.mouseInterface.reparentTo(self.mouseWatcher)
        self.mouse2cam.reparentTo(self.mouseInterface)

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
        else:
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
            numDisplayRegions = self.camNode.getNumDisplayRegions()
            for d in range(0, numDisplayRegions):
                dr = self.camNode.getDisplayRegion(d)
                dr.setCullFrustum(pnode)
        else:
            # Disable OOBE culling.

            # Assign each DisplayRegion shared by the camera to use
            # the default cull frustum, the camera itself.
            numDisplayRegions = self.camNode.getNumDisplayRegions()
            for d in range(0, numDisplayRegions):
                dr = self.camNode.getDisplayRegion(d)
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
        # Announce to anybody that a screenshot has been taken
        messenger.send('screenshot')

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


    def run(self):
        self.taskMgr.run()

