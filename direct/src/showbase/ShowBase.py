
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
import LinearEulerIntegrator
import ClockObject

globalClock = ClockObject.ClockObject.getGlobalClock()

class ShowBase:

    notify = None

    def __init__(self):

        # Get the dconfig object
        self.config = ConfigConfigureGetConfigConfigShowbase

        # Store dconfig variables
        self.wantTk = self.config.GetBool('want-tk', 0)
        self.wantSound = self.config.GetBool('want-sound', 1)
        self.wantDIRECT = self.config.GetBool('want-directtools', 0)
        self.wantStats = self.config.GetBool('want-stats', 0)

        import Loader

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
                                      self.dataRoot.node(),
                                      self.initialState)

        # This is a list of cams associated with the display region's cameras
        self.camList = []
        for camera in self.cameraList:
            self.camList.append( camera.find('**/+Camera') )
        # Set the default camera
        self.cam = self.camera.find('**/+Camera')

        # Set up a 2-d layer for drawing things behind Gui labels.
        self.render2d = NodePath(setupPanda2d(self.win, "render2d"))

        # Set up another 2-d layer for drawing the Gui labels themselves.
        self.renderGui = NodePath(setupPanda2d(self.win, "renderGui"))

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
        self.trackball = self.dataUnused.attachNewNode(Trackball('trackball'))
        self.drive = self.dataUnused.attachNewNode(DriveInterface('drive'))
        self.mouse2cam = self.dataUnused.attachNewNode(Transform2SG('mouse2cam'))
        self.mouse2cam.node().setArc(self.camera.getBottomArc())
        self.useDrive()

        self.buttonThrower = self.mouseWatcher.attachNewNode(ButtonThrower())

        if (ShowBase.notify == None):
            ShowBase.notify = directNotify.newCategory("ShowBase")

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
	self.integrator = LinearEulerIntegrator.LinearEulerIntegrator()
	self.physicsMgr.attachLinearIntegrator(self.integrator)
	self.physicsMgrEnabled = 0

	# Spawn the update managers task
	self.taskMgr.spawnTaskNamed(Task.Task(self.__updateManagers),
					'manager-update')

        self.createAudioManager()
        self.createRootPanel()
        self.createStats()

        self.restart()

    def enableParticles(self, flag = 1):
	"""enableParticles(self, flag)"""
	self.particleMgrEnabled = flag
	self.physicsMgrEnabled = flag

    def __updateManagers(self, state):
	"""__updateManagers(self)"""
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
            
    def createRootPanel(self):
        if self.wantTk:
            from TkGlobal import *
            self.tkroot = Pmw.initialise()
        else:
            self.tkroot = None

    def igloop(self, state):
        self.win.update()
        return Task.cont
    
    def restart(self):
        self.shutdown()
        self.taskMgr.spawnTaskNamed(Task.Task(self.igloop), 'igloop')
        self.eventMgr.restart()

    def shutdown(self):
        self.taskMgr.removeTasksNamed('igloop')
        self.eventMgr.shutdown()

    def toggleBackface(self):
        toggleBackface(self.initialState)

    def toggleTexture(self):
        toggleTexture(self.initialState)

    def toggleWireframe(self):
        toggleWireframe(self.initialState)

    def disableMouse(self):
        self.drive.reparentTo(self.dataUnused)
        self.trackball.reparentTo(self.dataUnused)
        self.mouse2cam.reparentTo(self.dataUnused)
        self.mouseInterface = None
        self.mouseInterfaceNode = None

    def setMouseOnArc(self, newArc):
        self.mouse2cam.node().setArc(newArc)

    def useDrive(self):
        """
        Toggle mouse action to drive mode
        """
        # Get rid of the trackball
        self.trackball.reparentTo(self.dataUnused)
        # Update the mouseInterface to point to the drive
        self.mouseInterface = self.drive
        self.mouseInterfaceNode = self.mouseInterface.getBottomNode()
        self.drive.node().reset()
        self.drive.reparentTo(self.mouseWatcher)
        # Hookup the drive to the camera
        self.mouse2cam.reparentTo(self.drive)
        # Set the height to a good eyeheight
        self.drive.node().setZ(4.0)

    def useTrackball(self):
        """
        Toggle mouse action to trackball mode
        """
        # Get rid of the drive
        self.drive.reparentTo(self.dataUnused)
        # Update the mouseInterface to point to the trackball
        self.mouseInterface = self.trackball
        self.mouseInterfaceNode = self.mouseInterface.getBottomNode()
        # Hookup the trackball to the camera
        self.trackball.reparentTo(self.mouseWatcher)
        self.mouse2cam.reparentTo(self.trackball)
        
    def run(self):
        self.taskMgr.run()

