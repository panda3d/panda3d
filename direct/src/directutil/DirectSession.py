from PandaObject import *
from DirectGeometry import *
from DirectSelection import *
from DirectCameraControl import *
import OnscreenText
import math

class DirectSession(PandaObject):
    
    def __init__(self):
        self.contextList = []
        self.iRayList = []
        for camera in base.cameraList:
            self.contextList.append(DisplayRegionContext(base.win, camera))
            self.iRayList.append(SelectionRay(camera))
        self.chanCenter = self.getChanData(0)

        self.cameraControls = DirectCameraControl(self)

        # Initialize the collection of selected nodePaths
        self.selectedNodePaths = {}
        self.selectedNodePath = None
        self.lastSelected = None

        self.readout = OnscreenText.OnscreenText( '', 0.1, -0.95 )
        # self.readout.textNode.setCardColor(0.5, 0.5, 0.5, 0.5)
        self.readout.reparentTo( hidden )

        self.createObjectHandles()
        self.useObjectHandles()
        
        self.createBBox()
        self.bboxList = []

        self.fControl = 0
        self.fAlt = 0
        self.fShift = 0
        self.in2DWidget = 0

        self.iRay = self.iRayList[0]
        self.hitPt = Point3(0.0)

        self.actionEvents = [('selectNodePath', self.selectNodePath),
                             ('deselectNodePath', self.deselectNodePath),
                             ('deselectAll', self.deselectAll),
                             ('highlightNodePath', self.highlightNodePath),
                             ('removeNodePath', self.removeNodePath),
                             ('in2DWidget', self.in2DWidget)]
        self.keyEvents = ['left', 'right', 'up', 'down',
                          'escape', 'space', 'delete',
                          'shift', 'shift-up', 'alt', 'alt-up',
                          'control', 'control-up',
                          'b', 'c', 'f', 'l', 't', 'v', 'w']

        self.mouseEvents = ['mouse1', 'mouse1-up',
                            'mouse2', 'mouse2-up',
                            'mouse3', 'mouse3-up']

    def selectNodePath(self, aNodePath, multiSelect = 0):
	self.lastSelected = aNodePath

	# Do nothing if nothing selected
        if not aNodePath:
            print 'Nothing selected!!'
            return 0

	# Reset selected objects and highlight if multiSelect is false
        if not multiSelect:
            self.deselectAll()

	# Record newly selected object
        # Use this pointer as an index
	self.selectedNodePaths[aNodePath.this] = aNodePath
        self.highlightNodePath(aNodePath)
	self.readout.reparentTo(render2d)
	self.readout.setText(self.getNodeName(aNodePath))

    def getNodeName(self, aNodePath):
        node = aNodePath.node()
        name = '<noname>'
        if issubclass(node.__class__, NamedNode):
            namableName = node.getName()
            if len(namableName) != 0:
                name = namableName
        return name

    def in2DWidget(self):
        self.in2DWidget = 1

    def deselectNodePath(self, aNodePath):
	# remove nodePath from selectedNodePaths dictionary if it exists
        key = aNodePath.this
        if self.selectedNodePaths.has_key(key):
            del self.selectedNodePaths[key]
	# Hide the manipulation widget
	self.objectHandles.reparentTo(hidden)
	self.readout.reparentTo(hidden)
	self.readout.setText(' ')
	taskMgr.removeTasksNamed('followSelectedNodePath')

    def deselectAll(self):
	self.selectedNodePaths = {}
	# Hide the manipulation widget
	self.objectHandles.reparentTo(hidden)
	self.readout.reparentTo(hidden)
	self.readout.setText(' ')
	taskMgr.removeTasksNamed('followSelectedNodePath')

    def highlightNodePath(self, aNodePath):
	selectedBounds = self.getBounds(aNodePath)
        # Does this work?
	radius = selectedBounds.getRadius()
	# radius = 5.0.
	# Place the manipulation widget on the object too
	self.objectHandles.reparentTo(render)
	self.objectHandles.setScale(radius)
        # Spawn task to have object handles follow the selected object
	taskMgr.removeTasksNamed('followSelectedNodePath')
        t = Task.Task(self.followSelectedNodePathTask)
        t.aNodePath = aNodePath
        taskMgr.spawnTaskNamed(t, 'followSelectedNodePath')

    def followSelectedNodePathTask(self, state):
        aNodePath = state.aNodePath
        pos = aNodePath.getPos(render)
        self.objectHandles.setPos(pos)
        return Task.cont

    def isolateSelected(self):
	selected = self.selectedNodePath
        if selected:
            self.showAllDescendants(selected.getParent())
            self.hideSiblings(selected)

    def removeNodePath(self, aNodePath):
	# Method to handle the remove event sent by the Scene Graph Explorer
	# Remove highlight and deselect nodePath
	self.deselectNodePath(aNodePath)
	# Send message in case anyone needs to do something
        # before node is deleted
	messenger.send('preRemoveNodePath', [aNodePath])
	# Remove nodePath
	aNodePath.reparentTo(hidden)
	aNodePath.removeNode()

    def removeSelectedNodePaths(self):
	# Remove all selected nodePaths from the Scene Graph
        for key in self.selectedNodePaths.keys():
            np = self.selectedNodePaths[key]
            self.removeNodePath(np)

    def toggleVizSelectedNodePaths(self):
        # Toggle visibility of selected node paths
        for key in self.selectedNodePaths.keys():
            path = self.selectedNodePaths[key]
            if path.isHidden():
                path.show()
            else:
                path.hide()

    def getBounds(self, aNodePath):
        # Get a node path's bounds
        nodeBounds = aNodePath.node().getBound()
        for kid in aNodePath.getChildrenAsList():
            nodeBounds.extendBy(kid.getBottomArc().getBound())
            return nodeBounds.makeCopy()

    def showAllDescendantsSelectedNodePath(self):
        # Show the descendants of the selectedNodePath
	selected = self.selectedNodePath
        if selected:
            self.showAllDescendants(selected)

    def showAllDescendants(self, aNodePath):
	aNodePath.show()
        for child in aNodePath.getChildrenAsList():
            self.showAllDescendants(child)

    def showSelectedNodePathSiblings(self):
	selected = self.selectedNodePath
        if selected:
            self.showSiblings(selected)

    def showSiblings(self, aNodePath):
	aNodePath.show()
        for sib in aNodePath.getParent().getChildrenAsList():
            if sib != aNodePath:
                sib.hide()

    def hideSelectedNodePathSiblings(self):
	selected = self.selectedNodePath
	if selected:
            self.hideSiblings(selected)

    def hideSiblings(self, aNodePath):
	aNodePath.show()
        for sib in aNodePath.getParent().getChildrenAsList():
            if sib != aNodePath:
                sib.hide()

    def enable(self):
	# Start all display region context tasks
        for context in self.contextList:
            context.spawnContextTask()
	# Turn on mouse Flying
	self.cameraControls.enableMouseFly()
	# Accept appropriate hooks
	self.enableKeyEvents()
	self.enableMouseEvents()
	self.enableActionEvents()

    def disable(self):
	# Shut down all display region context tasks
        for context in self.contextList:
            context.removeContextTask()
	# Turn off camera fly
	self.cameraControls.disableMouseFly()
	self.disableKeyEvents()
	self.disableMouseEvents()
	self.disableActionEvents()

    def minimumConfiguration(self):
	# Remove context task
	for context in self.contextList:
            context.removeContextTask()
	# Turn off camera fly
	self.cameraControls.disableMouseFly()
	# Ignore keyboard and action events
	self.disableKeyEvents()
	self.disableActionEvents()
	# But let mouse events pass through
	self.enableMouseEvents()

    def destroy(self):
	self.disable()

    def restart(self):
	self.enable()

    def enableActionEvents(self):
        for event, method in self.actionEvents:
            self.accept(event, method)

    def enableKeyEvents(self):
        for event in self.keyEvents:
            self.accept(event, self.inputHandler, [event])

    def enableMouseEvents(self):
        for event in self.mouseEvents:
            self.accept(event, self.inputHandler, [event])

    def disableActionEvents(self):
        for event in self.actionEvents:
            self.ignore(event)

    def disableKeyEvents(self):
        for event in self.keyEvents:
            self.ignore(event)

    def disableMouseEvents(self):
        for event in self.mouseEvents:
            self.ignore(event)

    def useObjectHandles(self):
        self.widget = self.objectHandles

    def hideReadout(self):
	self.readout.reparentTo(hidden)

    def getChanData(self, index):
        return self.contextList[index]

    def inputHandler(self, input):
	# Deal with keyboard and mouse input
	if ((input != 'mouse1-up') & (input != 'mouse2-up') &
            (input != 'mouse3-up')):
            self.in2DWidget = 0
            
        if input == 'mouse1':
            messenger.send('handle2DMouse1')
            if not self.in2DWidget:
                messenger.send('handleMouse1')
        elif input == 'mouse1-up':
            messenger.send('handle2DMouse1Up')
            if not self.in2DWidget:
                messenger.send('handleMouse1Up')
        elif input == 'mouse3': 
            messenger.send('handleMouse3')
        elif input == 'mouse3-up':
            messenger.send('handleMouse3Up')
        elif input == 'shift':
            self.fShift = true
        elif input == 'shift-up':
            self.fShift = false
        elif input == 'control':
            self.fControl = true
        elif input == 'control-up':
            self.fControl = false
        elif input == 'alt':
            self.fAlt = true
        elif input == 'alt-up':
            self.fAlt = false
        elif input == 'escape':
            self.deselectAll()
        elif input == 'l':
            if not self.lastSelected:
                self.selectNodePath(self.lastSelected)
        elif input == 'delete':
            self.removeSelectedNodePaths()
        elif input == 'v':
            self.toggleVizSelectedNodePaths()
        elif input == 'b':
            base.toggleBackface()
        elif input == 't':
            base.toggleTexture()
        elif input == 'w':
            base.toggleWireframe()

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
	oh = self.objectHandles = hidden.attachNewNode(
            NamedNode('objectHandles') )
	ohLines = LineNodePath( oh )
	ohLines.setColor( VBase4( 1.0, 0.0, 1.0, 1.0) )
	ohLines.setThickness( 3.0 )

	# InnerRing
	ohLines.moveTo( 0.8, 0.0, 0.0 )
        for ang in range(10, 360, 10):
            ohLines.drawTo( (0.8 * math.cos(deg2Rad(ang))),
                            (0.8 * math.sin(deg2Rad(ang))),
                            0.0 )

	# Outer Ring 
	ohLines.moveTo( 1.2, 0.0, 0.0 )
        for ang in range(0, 360, 10):
            ohLines.drawTo( (1.2 * math.cos(deg2Rad(ang))),
                            (1.2 * math.sin(deg2Rad(ang))),
                            0.0 )

	ohLines.moveTo( 0.0, 0.0, 0.0 )
	ohLines.drawTo( 0.0, 0.0, 1.5 )
        # Create the line segments
	ohLines.create()
        
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

    def removeContextTask(self):
        taskMgr.removeTasksNamed('DIRECTContextTask')

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


