from PandaObject import *
from DirectCameraControl import *
from DirectManipulation import *
from DirectSelection import *
from DirectGrid import *
from DirectGeometry import *
import OnscreenText


class DirectSession(PandaObject):

    def __init__(self):
        self.contextList = []
        self.iRayList = []
        for camera in base.cameraList:
            self.contextList.append(DisplayRegionContext(base.win, camera))
            self.iRayList.append(SelectionRay(camera))
        self.chan = self.getChanData(0)
        self.camera = base.cameraList[0]

        self.cameraControl = DirectCameraControl(self)
        self.manipulationControl = DirectManipulationControl(self)
        self.useObjectHandles()
        self.grid = DirectGrid(self)
        self.grid.disable()

        # Initialize the collection of selected nodePaths
        self.selected = SelectedNodePaths(self)

        self.readout = OnscreenText.OnscreenText( '', 0.1, -0.95 )
        # self.readout.textNode.setCardColor(0.5, 0.5, 0.5, 0.5)
        self.readout.reparentTo( hidden )

        self.fControl = 0
        self.fAlt = 0
        self.fShift = 0
        self.in2DWidget = 0

        self.pos = VBase3()
        self.hpr = VBase3()
        self.scale = VBase3()

        self.iRay = self.iRayList[0]
        self.hitPt = Point3(0.0)

        # One run through the context task to init everything
        for context in self.contextList:
            context.contextTask(None)

        self.actionEvents = [('select', self.select),
                             ('deselect', self.deselect),
                             ('deselectAll', self.deselectAll),
                             ('highlightAll', self.selected.highlightAll),
                             ('preRemoveNodePath', self.deselect),
                             ('in2DWidget', self.in2DWidget)]
        self.keyEvents = ['left', 'right', 'up', 'down',
                          'escape', 'space', 'delete',
                          'shift', 'shift-up', 'alt', 'alt-up',
                          'control', 'control-up',
                          'b', 'c', 'f', 'l', 't', 'v', 'w']
        self.mouseEvents = ['mouse1', 'mouse1-up',
                            'mouse2', 'mouse2-up',
                            'mouse3', 'mouse3-up']

    def select(self, nodePath):
        dnp = self.selected.select(nodePath)
        if dnp:
            # Update the readout
            self.readout.reparentTo(render2d)
            self.readout.setText(dnp.name)
            # Show the manipulation widget
            self.widget.reparentTo(render)

            # Update camera controls coa to this point
            # Coa2Camera = Coa2Dnp * Dnp2Camera
            mCoa2Camera = dnp.mCoa2Dnp * dnp.getMat(base.camera)
            row = mCoa2Camera.getRow(3)
            coa = Vec3(row[0], row[1], row[2])
            self.cameraControl.updateCoa(coa)

            # Adjust widgets size
            # This uses the additional scaling factor used to grow and
            # shrink the widget            
            self.widget.setScalingFactor(dnp.getRadius())

            # Spawn task to have object handles follow the selected object
            taskMgr.removeTasksNamed('followSelectedNodePath')
            t = Task.Task(self.followSelectedNodePathTask)
            t.dnp = dnp
            taskMgr.spawnTaskNamed(t, 'followSelectedNodePath')
            # Send an message marking the event
            messenger.send('selectedNodePath', [dnp])

    def followSelectedNodePathTask(self, state):
        mCoa2Render = state.dnp.mCoa2Dnp * state.dnp.getMat(render)
        decomposeMatrix(mCoa2Render,
                        self.scale,self.hpr,self.pos,
                        getDefaultCoordinateSystem())
        self.widget.setPosHpr(self.pos,self.hpr)
        return Task.cont

    def deselect(self, nodePath):
        dnp = self.selected.deselect(nodePath)
        if dnp:
            # Hide the manipulation widget
            self.widget.reparentTo(hidden)
            self.readout.reparentTo(hidden)
            self.readout.setText(' ')
            taskMgr.removeTasksNamed('followSelectedNodePath')
            # Send an message marking the event
            messenger.send('deselectedNodePath', [dnp])

    def deselectAll(self):
        self.selected.deselectAll()
        # Hide the manipulation widget
        self.widget.reparentTo(hidden)
        self.readout.reparentTo(hidden)
        self.readout.setText(' ')
        taskMgr.removeTasksNamed('followSelectedNodePath')

    def in2DWidget(self):
        self.in2DWidget = 1

    def enable(self):
        # Make sure old tasks are shut down
        self.disable()
	# Start all display region context tasks
        for context in self.contextList:
            context.spawnContextTask()
	# Turn on mouse Flying
	self.cameraControl.enableMouseFly()
        # Turn on object manipulation
        self.manipulationControl.enableManipulation()
	# Accept appropriate hooks
	self.enableKeyEvents()
	self.enableMouseEvents()
	self.enableActionEvents()

    def disable(self):
	# Shut down all display region context tasks
        for context in self.contextList:
            context.removeContextTask()
	# Turn off camera fly
	self.cameraControl.disableMouseFly()
        # Turn off object manipulation
        self.manipulationControl.disableManipulation()
	self.disableKeyEvents()
	self.disableMouseEvents()
	self.disableActionEvents()

    def minimumConfiguration(self):
	# Remove context task
	for context in self.contextList:
            context.removeContextTask()
	# Turn off camera fly
	self.cameraControl.disableMouseFly()
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
        for event, method in self.actionEvents:
            self.ignore(event)

    def disableKeyEvents(self):
        for event in self.keyEvents:
            self.ignore(event)

    def disableMouseEvents(self):
        for event in self.mouseEvents:
            self.ignore(event)

    def useObjectHandles(self):
        self.widget = self.manipulationControl.objectHandles

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
        elif input == 'mouse2': 
            messenger.send('handleMouse2')
        elif input == 'mouse2-up':
            messenger.send('handleMouse2Up')
        elif input == 'mouse3': 
            messenger.send('handleMouse3')
        elif input == 'mouse3-up':
            messenger.send('handleMouse3Up')
        elif input == 'shift':
            self.fShift = 1
        elif input == 'shift-up':
            self.fShift = 0
        elif input == 'control':
            self.fControl = 1
        elif input == 'control-up':
            self.fControl = 0
        elif input == 'alt':
            self.fAlt = 1
        elif input == 'alt-up':
            self.fAlt = 0
        elif input == 'escape':
            self.deselectAll()
        elif input == 'l':
            if self.selected.last:
                self.select(self.selected.last)
        elif input == 'delete':
            self.selected.removeAll()
        elif input == 'v':
            self.selected.toggleVizAll()
        elif input == 'b':
            base.toggleBackface()
        elif input == 't':
            base.toggleTexture()
        elif input == 'w':
            base.toggleWireframe()
        
class DisplayRegionContext(PandaObject):
    def __init__(self, win, camera):
        self.win = win
        self.camera = camera
        self.cam = camera.getChild(0)
        self.camNode = self.cam.getNode(0)
        self.mouseData = win.getMouseData(0)
        self.nearVec = Vec3(0)
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
        self.left = -self.nearWidth/2.0
        self.right = self.nearWidth/2.0
        self.top = self.nearHeight/2.0
        self.bottom = -self.nearHeight/2.0
        # Mouse Data
        # Last frame
        self.mouseLastX = self.mouseX
        self.mouseLastY = self.mouseY
        # Values for this frame
        # Pixel coordinates of the mouse
        self.mousePixelX = self.mouseData.getX()
        self.mousePixelY = self.mouseData.getY()
        # This ranges from -1 to 1
        self.mouseX = ((self.mousePixelX / float(self.width)) * 2.0) - 1.0
        self.mouseY = ((self.mousePixelY / float(self.height)) * -2.0) + 1.0
        # Delta percent of window the mouse moved
        self.mouseDeltaX = self.mouseX - self.mouseLastX
        self.mouseDeltaY = self.mouseY - self.mouseLastY
        self.nearVec.set((self.nearWidth/2.0) * self.mouseX,
                         self.near,
                         (self.nearHeight/2.0) * self.mouseY)
        # Continue the task
        return Task.cont

