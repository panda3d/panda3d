
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
import Transitions
import Loader
import time
import FSM
import State

globalClock = ClockObject.getGlobalClock()

class ShowBase:

    notify = directNotify.newCategory("ShowBase")

    def __init__(self):

        # Get the dconfig object
        self.config = ConfigConfigureGetConfigConfigShowbase

        # Store dconfig variables
        self.wantTk = self.config.GetBool('want-tk', 0)
        self.wantSound = self.config.GetBool('want-sound', 1)
        if not self.wantSound:
            AudioManager.setAllSoundActive(0)
        self.wantSfx = AudioManager.getSfxActive()
        self.wantMusic = AudioManager.getMusicActive()
        if not (self.wantSfx or self.wantMusic):
            self.wantSound = None
        self.wantDIRECT = self.config.GetBool('want-directtools', 0)
        self.wantStats = self.config.GetBool('want-stats', 0)

        taskMgr.taskTimerVerbose = self.config.GetBool('task-timer-verbose', 0)

        fsmRedefine = self.config.GetBool('fsm-redefine', 0)
        State.FsmRedefine = fsmRedefine

        self.initialState = NodeAttributes()
        # Set a default "off color" (i.e. use poly color) for color transitions
        self.initialState.setAttribute(ColorTransition.getClassType(),
                                       ColorAttribute())
        self.renderTop = NodePath(NamedNode('renderTop'))
        self.render = self.renderTop.attachNewNode('render')
        self.hidden = NodePath(NamedNode('hidden'))
        # This will be the list of cameras, one per display region
        # For now, we only have one display region, so just create the
        # default camera
        self.camera = self.render.attachNewNode('camera')
        # And put it in the list
        self.cameraList = [ self.camera ]
        self.dataRoot = NodePath(NamedNode('dataRoot'), DataRelation.getClassType())
        self.dataUnused = NodePath(NamedNode('dataUnused'), DataRelation.getClassType())
        self.pipe = makeGraphicsPipe()
        self.win = makeGraphicsWindow(self.pipe,
                                      self.renderTop.node(),
                                      self.camera.node(),
                                      self.initialState)

        # This is a placeholder for a CollisionTraverser.  If someone
        # stores a CollisionTraverser pointer here, we'll traverse it
        # in the igloop task.
        self.cTrav = 0

        # This is a list of cams associated with the display region's cameras
        self.camList = []
        for camera in self.cameraList:
            self.camList.append( camera.find('**/+Camera') )
        # Set the default camera
        self.cam = self.camera.find('**/+Camera')

        # Set up a 2-d layer for drawing things behind Gui labels.
        self.render2d = NodePath(setupPanda2d(self.win, "render2d"))

        # The normal 2-d layer has an aspect ratio that matches the
        # window, but its coordinate system is square.  This means
        # anything we parent to render2d gets stretched.  For things
        # where that makes a difference, we set up aspect2d, which
        # scales things back to the right aspect ratio.

        # For now, we assume that the window will have an aspect ratio
        # matching that of a traditional PC screen.
        self.aspectRatio = 4.0 / 3.0
        self.aspect2d = self.render2d.attachNewNode("aspect2d")
        self.aspect2d.setScale(1.0 / self.aspectRatio, 1.0, 1.0)

        # It's important to know the bounds of the aspect2d screen.
        self.a2dTop = 1.0
        self.a2dBottom = -1.0
        self.a2dLeft = -self.aspectRatio
        self.a2dRight = self.aspectRatio

        # Set up an auxiliary 3-d layer for rendering floating heads
        # or other 3-d objects on top of text or widgets in the 2-d
        # layer.  We set it up with a camera that specifically shares
        # the projection with the default camera, so that when we
        # change the default camera's parameters, it changes this one
        # too.
        self.renderAux = NodePath(NamedNode('renderAux'))
        self.camAux = self.renderAux.attachNewNode(Camera('camAux'))
        self.camAux.node().shareProjection(self.cam.node().getProjection())
        addRenderLayer(self.win, self.renderAux.node(), self.camAux.node())

        # We create both a MouseAndKeyboard object and a MouseWatcher object
        # for the window.  The MouseAndKeyboard generates mouse events and
        # mouse button/keyboard events; the MouseWatcher passes them through
        # unchanged when the mouse is not over a 2-d button, and passes
        # nothing through when the mouse *is* over a 2-d button.  Therefore,
        # objects that don't want to get events when the mouse is over a
        # button, like the driveInterface, should be parented to
        # mouseWatcher, while objects that want events in all cases, like the
        # chat interface, should be parented to mak.
        self.mak = self.dataRoot.attachNewNode(MouseAndKeyboard(self.win, 0, 'mak'))
        self.mouseWatcher = self.mak.attachNewNode(MouseWatcher('mouseWatcher'))

        # We also create a DataValve object above the trackball/drive
        # interface, which will allow us to switch some of the mouse
        # control, without switching all of it, to another object
        # later (for instance, to enable OOBE mode--see oobe(),
        # below.)
        self.mouseValve = self.mouseWatcher.attachNewNode(DataValve('mouseValve'))
        # This Control object can be used to turn on and off mouse &
        # keyboard messages to the DriveInterface.
        self.mouseControl = DataValve.Control()
        self.mouseValve.node().setControl(0, self.mouseControl)

        # This Control object is always kept on, handy to have.
        self.onControl = DataValve.Control()

        # Now we have the main trackball & drive interfaces.
        # useTrackball() and useDrive() switch these in and out; only
        # one is in use at a given time.
        self.trackball = self.dataUnused.attachNewNode(Trackball('trackball'))
        self.drive = self.dataUnused.attachNewNode(DriveInterface('drive'))
        self.mouse2cam = self.dataUnused.attachNewNode(Transform2SG('mouse2cam'))
        self.mouse2cam.node().setArc(self.camera.arc())
        self.useDrive()

        self.buttonThrower = self.mouseWatcher.attachNewNode(ButtonThrower())

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

        # Transition effects (fade, iris, etc)
        self.transitions = Transitions.Transitions(self.loader)

        # Tk
        if self.wantTk:
            import TkGlobal

        self.restart()

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
	self.taskMgr.removeTasksNamed('manager-update')
	self.taskMgr.spawnTaskNamed(Task.Task(self.updateManagers),
					'manager-update')

    def disableParticles(self):
	"""enableParticles(self)"""
	self.particleMgrEnabled = 0
	self.physicsMgrEnabled = 0
	self.taskMgr.removeTasksNamed('manager-update')

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
            PStatClient.getGlobalPstats().connect()

    def createAudioManager(self):
        if self.wantSound:
            AudioManager.spawnUpdate()

    def loadSfx(self, name):
        if name:
            if base.wantSfx:
                s = loader.loadSound(name)
                return s
        return None

    def loadMusic(self, name):
        if name:
            if base.wantMusic:
                m = loader.loadSound(name)
                return m
        return None

    def unloadSfx(self, sfx):
        if sfx:
            loader.unloadSound(sfx)

    def unloadMusic(self, music):
        if music:
            loader.unloadSound(music)

    def playSfx(self, sfx, looping = None, interupt = 1, volume = None):
        if sfx:
            if base.wantSfx:
                if not interupt:
                    if not (sfx.status() == AudioSound.PLAYING):
                        AudioManager.play(sfx)
                else:
                    AudioManager.play(sfx)
                if looping:
                    AudioManager.setLoop(sfx, 1)
                if volume:
                    AudioManager.setVolume(sfx, volume)

    def playMusic(self, music, looping = None, interupt = 1, volume = None,
                  restart = None):
        if music:
            if base.wantMusic:
                if not interupt:
                    if not (music.status() == AudioSound.PLAYING):
                        AudioManager.plsy(music)
                else:
                    AudioManager.play(music)
                if looping:
                    AudioManager.setLoop(music, 1)
                if volume:
                    AudioManager.setVolume(music, volume)
                if restart:
                    restart[0].accept("restart-music", restart[1])

    def stopSfx(self, sfx):
        if sfx:
            if base.wantSfx:
                AudioManager.stop(sfx)

    def stopMusic(self, music, restart = None):
        if music:
            if base.wantMusic:
                AudioManager.stop(music)
                if restart:
                    restart[0].ignore("restart-music")

    def dataloop(self, state):
        # traverse the data graph.  This reads all the control
        # inputs (from the mouse and keyboard, for instance) and also
        # directly acts upon them (for instance, to move the avatar).
        directTraverseDataGraph(self.dataRoot.node())

        return Task.cont

    def igloop(self, state):
        # run the collision traversal if we have a
        # CollisionTraverser set.
        if self.cTrav:
            self.cTrav.traverse(self.render)

        # Finally, render the frame.
        self.win.update()

        return Task.cont

    def restart(self):
        self.shutdown()
        # give the igloop task a reasonably "late" priority,
        # so that it will get run after most tasks
        self.taskMgr.spawnTaskNamed(Task.Task(self.igloop, 50), 'igloop')
        # give the dataloop task a reasonably "early" priority,
        # so that it will get run before most tasks
        self.taskMgr.spawnTaskNamed(Task.Task(self.dataloop, -50), 'dataloop')
        self.eventMgr.restart()

    def shutdown(self):
        self.taskMgr.removeTasksNamed('igloop')
        self.taskMgr.removeTasksNamed('dataloop')
        self.eventMgr.shutdown()

    def toggleBackface(self):
        toggleBackface(self.initialState)

    def toggleTexture(self):
        toggleTexture(self.initialState)

    def toggleWireframe(self):
        toggleWireframe(self.initialState)

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
        # Hookup the drive to the camera.  Make sure it is first in
        # the list of children of the mouseValve.
        self.drive.reparentTo(self.mouseValve, 0)
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
        # Hookup the trackball to the camera.  Make sure it is first
        # in the list of children of the mouseValve.
        self.trackball.reparentTo(self.mouseValve, 0)
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
            self.oobeControl = DataValve.Control()
            self.mouseValve.node().setControl(1, self.oobeControl)
            self.oobeTrackball = self.mouseValve.attachNewNode(Trackball('oobeTrackball'), 1)
            self.oobe2cam = self.oobeTrackball.attachNewNode(Transform2SG('oobe2cam'))
            self.oobe2cam.node().setArc(self.oobeCameraTrackball.arc())

            self.oobeButtonEventsType = TypeRegistry.ptr().findType('ButtonEvents_ButtonEventDataTransition')

            self.oobeVis = loader.loadModelOnce('models/misc/camera')

            # Make sure the MouseValve is monitoring the Control key.
            mods = ModifierButtons(self.mouseValve.node().getModifierButtons())
            mods.addButton(KeyboardButton.control())
            self.mouseValve.node().setModifierButtons(mods)

        if self.oobeMode:
            # Disable OOBE mode.
            self.oobeControl.setOff()
            self.mouseControl.setOn()
            if self.oobeVis:
                self.oobeVis.reparentTo(self.hidden)
            self.cam.reparentTo(self.camera)
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
            cameraParent = NodePath(self.camera)
            cameraParent.shorten(1)
            self.oobeCamera.reparentTo(cameraParent)
            self.oobeCamera.clearMat()

            # Set our initial OOB position to be just behind the camera.
            mat = Mat4.translateMat(0, -10, 3) * base.camera.getMat(cameraParent)
            mat.invertInPlace()
            self.oobeTrackball.node().setMat(mat)

            self.cam.reparentTo(self.oobeCameraTrackball)
            if self.oobeVis:
                self.oobeVis.reparentTo(self.camera)
            self.oobeMode = 1

    def screenshot(self, namePrefix='screenshot'):
        # Get the current date and time to uniquify the image (down to the second)
        date = time.ctime(time.time())
        # Get the current frame count to uniqify it even more
        frameCount = globalClock.getFrameCount()
        # Replace spaces with dashes because unix does not like spaces in the filename
        date = date.replace(' ', '-')
        date = date.replace(':', '-')
        imageName = (namePrefix + '-' + date + '-' + str(frameCount) + '.pnm')
        self.notify.info("Taking screenshot: " + imageName)
        takeSnapshot(self.win, imageName)

    def run(self):
        self.taskMgr.run()

