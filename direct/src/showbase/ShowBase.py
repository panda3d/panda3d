
from PandaModules import *
from DirectNotifyGlobal import *
from MessengerGlobal import *
from TaskManagerGlobal import *
from EventManagerGlobal import *
from PythonUtil import *
import Task
import EventManager
import math
import sys

class ShowBase:

    notify = None

    def __init__(self):

        # Get the dconfig object
        self.config = ConfigConfigureGetConfigConfigShowbase

        # Store dconfig variables
        self.wantTk = self.config.GetBool('want-tk', 0)
        self.wantSound = self.config.GetBool('want-sound', 1)
        self.wantDIRECT = self.config.GetBool('want-directtools', 0)
        
        import Loader

        self.initialState = NodeAttributes()
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
        self.win = self.pipe.makeGraphicsWindow(self.renderTop.node(),
                                                self.camera.node(),
                                                self.dataRoot.node(),
                                                self.initialState)

        self.render2d = NodePath(self.win.setupPanda2d())
        # This is a list of cams associated with the display region's cameras
        self.camList = []
        for camera in self.cameraList:
            self.camList.append( camera.find('**/+Camera') )
        # Set the default camera
        self.cam = self.camera.find('**/+Camera')
        # Just one per window
        self.mak = self.dataRoot.attachNewNode(MouseAndKeyboard(self.win, 0, 'mak'))
        self.trackball = self.dataUnused.attachNewNode(Trackball('trackball'))
        self.drive = self.dataUnused.attachNewNode(DriveInterface('drive'))
        self.mouse2cam = self.dataUnused.attachNewNode(Transform2SG('mouse2cam'))
        self.mouse2cam.node().setArc(self.camera.getBottomArc())
        self.useDrive()

        self.buttonThrower = self.mak.attachNewNode(ButtonThrower())

        if (ShowBase.notify == None):
            ShowBase.notify = directNotify.newCategory("ShowBase")

        self.loader = Loader.Loader(self)

        self.eventMgr = eventMgr

        self.messenger = messenger

        self.taskMgr = taskMgr

        self.createAudioManager()
        self.createRootPanel()

        self.restart()

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
        #self.shutdown()
        self.taskMgr.spawnTaskNamed(Task.Task(self.igloop), 'igloop')
        self.eventMgr.restart()

    def shutdown(self):
        self.taskMgr.removeTasksNamed('igloop')
        self.eventMgr.shutdown()

    def toggleBackface(self):
        self.initialState.toggleBackface()

    def toggleTexture(self):
        self.initialState.toggleTexture()

    def toggleWireframe(self):
        self.initialState.toggleWireframe()

    def disableMouse(self):
        self.drive.reparentTo(self.dataUnused)
        self.trackball.reparentTo(self.dataUnused)
        self.mouse2cam.reparentTo(self.dataUnused)
        self.mouseInterface = None

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
        self.drive.node().reset()
        self.drive.reparentTo(self.mak)
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
        # Hookup the trackball to the camera
        self.trackball.reparentTo(self.mak)
        self.mouse2cam.reparentTo(self.trackball)
        
    def run(self):
        self.taskMgr.run()

if __name__ == '__main__':
    base = ShowBase()
    base.run()

