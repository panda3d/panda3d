from PandaObject import *
from DirectGeometry import *
import OnscreenText
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

        self.readout = OnscreenText.OnscreenText( '', 0.1, -0.95 )
        # self.readout.textNode.setCardColor(0.5, 0.5, 0.5, 0.5)
        self.readout.reparentTo( hidden )

        self.createBBox()

        # self.createObjectHandles()
        # self.useObjectHandles()
        
        self.bboxList = []

        self.fControl = 0
        self.fAlt = 0
        self.fShift = 0
        
    def createBBox(self, parent = hidden):
        # Create a line segments object for the bbox
	bbox = self.bbox = LineNodePath(parent)
	#bbox.setName('bbox')
        bbox.setColor( VBase4( 1., 0., 0., 1. ) )
	bbox.setThickness( 0.5 )

        # Bottom face
	bbox.drawTo( 0.0, 0.0, 0.0 )
	bbox.drawTo( 1.0, 0.0, 0.0 )
	bbox.drawTo( 1.0, 1.0, 0.0 )
	bbox.drawTo( 0.0, 1.0, 0.0 )
	bbox.drawTo( 0.0, 0.0, 0.0 )

	# Front Edge/Top face
	bbox.drawTo( 0.0, 0.0, 1.0 )
	bbox.drawTo( 1.0, 0.0, 1.0 )
	bbox.drawTo( 1.0, 1.0, 1.0 )
	bbox.drawTo( 0.0, 1.0, 1.0 )
	bbox.drawTo( 0.0, 0.0, 1.0 )

	# Three remaining edges
	bbox.moveTo( Point3( 1.0, 0.0, 0.0 ) )
	bbox.drawTo( 1.0, 0.0, 1.0 )
	bbox.moveTo( Point3( 1.0, 1.0, 0.0 ) )
	bbox.drawTo( 1.0, 1.0, 1.0 )
	bbox.moveTo( Point3( 0.0, 1.0, 0.0 ) )
	bbox.drawTo( 0.0, 1.0, 1.0 )

	bbox.create()

    def createObjectHandles(self):
	oh = self.objectHandles = hidden.attachNewNode( NamedNode('objectHandles') )
	ohLines = LineNodePath( oh )
	ohLines.setColor( VBase4( 1.0, 0.0, 1.0, 1.0) )
	ohLines.setThickness( 3.0 )

	"InnerRing"
	ohLines.moveTo( 0.8, 0.0, 0.0 )
        for ang in range(10, 360, 10):
            ohLines.drawTo( (0.8 * math.cos(deg2Rad(ang))),
                            (0.8 * math.sin(deg2Rad(ang))),
                            0.0 )

	"Outer Ring" 
	ohLines.moveTo( 1.2, 0.0, 0.0 )
        for ang in range(0, 360, 10):
            ohLines.drawTo( (1.2 * math.cos(deg2Rad(ang))),
                            (1.2 * math.sin(deg2Rad(ang))),
                            0.0 )

	ohLines.moveTo( 0.0, 0.0, 0.0 )
	ohLines.drawTo( 0.0, 0.0, 1.5 )

	ohLines.create()


        




