from PandaObject import *
import math

class DisplayRegionContext(PandaObject):
    def __init__(self, win, camera):
        self.win = win
        self.camera = camera
        self.cam = camera.getChild(0)
        self.camNode = self.cam.getNode(0)
        self.mouseData = win.getMouseData(0)
        self.mouseX = 0.0
        self.mouseY = 0.0

    def __getitem__(self,key):
        return self.__dict__[key]

    def start(self):
        # First shutdown any existing task
        self.stop()
        # Start a new context task
        self.spawnContextTask()

    def stop(self):
        # Kill the existing context task
        taskMgr.removeTasksNamed('DIRECTContextTask')

    def spawnContextTask(self):
        taskMgr.spawnTaskNamed(Task.Task(self.contextTask),
                               'DIRECTContextTask')

    def contextTask(self, state):
        # Window Data
        self.width = self.win.getWidth()
        self.height = self.win.getHeight()
        self.near = self.camNode.getNear()
        self.far = self.camNode.getFar()
        self.fovH = self.camNode.getHfov()
        self.fovV = self.camNode.getVfov()
        self.nearWidth = math.tan(deg2Rad(self.fovH / 2.0)) * self.near * 2.0
        self.nearHeight = math.tan(deg2Rad(self.fovV / 2.0)) * self.near * 2.0
        # Mouse Data
        # Last frame
        self.mouseLastX = self.mouseX
        self.mouseLastY = self.mouseY
        # This frame
        self.mousePixelX = self.mouseData.getX()
        self.mousePixelY = self.mouseData.getY()
        self.mouseX = ((self.mousePixelX / float(self.width)) * 2.0) - 1.0
        self.mouseY = ((self.mousePixelY / float(self.height)) * -2.0) + 1.0
        self.mouseDeltaX = self.mouseX - self.mouseLastX
        self.mouseDeltaY = self.mouseY - self.mouseLastY
        # Continue the task
        return Task.cont
        
class DirectSession(PandaObject):
    def __init__(self):
        self.contextList = []
        for camera in base.cameraList:
            self.contextList.append(DisplayRegionContext(base.win, camera))

        # Initialize the collection of selected nodePaths
        self.selectedNodePaths = {}
        self.lastSelected = None

        self.bboxList = []

        self.fControl = 0
        self.fAlt = 0
        self.fShift = 0
        
        """"
    def createBBox(self):
	bbox = hidden.attachNewNode(NamedNode())
	bbox.setName('bbox')
	bboxLines = GridLine new: bbox.
	bboxLines color: (VBase4 new: 1.0 y: 0.0 z: 0.0 w: 1.0).
	bboxLines thickness: 0.5.

	"Bottom face"
	bboxLines moveTo: 0.0 y: 0.0 z: 0.0.
	bboxLines drawTo: 1.0 y: 0.0 z: 0.0.
	bboxLines drawTo: 1.0 y: 1.0 z: 0.0.
	bboxLines drawTo: 0.0 y: 1.0 z: 0.0.
	bboxLines drawTo: 0.0 y: 0.0 z: 0.0.

	"Front Edge/Top face"
	bboxLines drawTo: 0.0 y: 0.0 z: 1.0.
	bboxLines drawTo: 1.0 y: 0.0 z: 1.0.
	bboxLines drawTo: 1.0 y: 1.0 z: 1.0.
	bboxLines drawTo: 0.0 y: 1.0 z: 1.0.
	bboxLines drawTo: 0.0 y: 0.0 z: 1.0.

	"Three remaining edges"
	bboxLines moveTo: 1.0 y: 0.0 z: 0.0.
	bboxLines drawTo: 1.0 y: 0.0 z: 1.0.
	bboxLines moveTo: 1.0 y: 1.0 z: 0.0.
	bboxLines drawTo: 1.0 y: 1.0 z: 1.0.
	bboxLines moveTo: 0.0 y: 1.0 z: 0.0.
	bboxLines drawTo: 0.0 y: 1.0 z: 1.0.

	bboxLines create: bboxLines lineNode.! !
        """

class Line(LineSegs):
    def __init__(self):
        LineSegs.__init__(self)





