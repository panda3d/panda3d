
from PandaModules import *
from DirectNotifyGlobal import *
from MessengerGlobal import *
from TaskManagerGlobal import *
from EventManagerGlobal import *
from AudioManagerGlobal import *
# This should be on a dconfig variable
from TkGlobal import *
import Task
import EventManager

class ShowBase:

    notify = None

    def __init__(self):
        
        import Loader

        self.initialState = NodeAttributes()
        self.renderTop = NodePath(NamedNode('renderTop'))
        self.render = self.renderTop.attachNewNode('render')
        self.hidden = NodePath(NamedNode('hidden'))
        self.camera = self.render.attachNewNode('camera')
        self.dataRoot = NodePath(NamedNode('dataRoot'), DataRelation.getClassType())
        self.dataUnused = NodePath(NamedNode('dataUnused'), DataRelation.getClassType())

        self.pipe = makeGraphicsPipe()
        self.win = self.pipe.makeGraphicsWindow(self.renderTop.node(),
                                                self.camera.node(),
                                                self.dataRoot.node(),
                                                self.initialState)

        self.render2d = NodePath(self.win.setupPanda2d())
        self.cam = self.camera.find('**/+Camera')
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

        self.audioMgr = audioMgr

        self.wantTk = 1
        self.createRootPanel()
        
        self.restart()

        self.wantSound = 1

        self.wantMusic = 1

    def createRootPanel(self):
        if self.wantTk:
            self.tkroot = Pmw.initialise()
        else:
            self.tkroot = None

    def igloop(self, state):
        self.win.update()
        if (self.wantSound):
            self.audioMgr.update()
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

