from PandaObject import *
from DirectCameraControl import *
from DirectSelection import *
from DirectGeometry import *
import OnscreenText

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

    def createBBox(self, parent = None):
        # Create a line segments object for the bbox
        if parent is None:
            parent = hidden
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


CAM_MOVE_DURATION = 1.0

class DirectCameraControl(PandaObject):
    def __init__(self, direct):
        # Create the grid
        self.direct = direct
        self.defChan = direct.chanCenter
	self.orthoViewRoll = 0.0
	self.lastView = 0
        self.coa = Point3(0)
	self.relNodePath = render.attachNewNode(NamedNode('targetNode'))
        self.zeroBaseVec = VBase3(0)
        self.zeroVector = Vec3(0)
        self.centerVec = Vec3(0, 1, 0)
        self.zeroPoint = Point3(0)

    def mouseFlyStart(self, chan):
	# Record starting mouse positions
	self.initMouseX = chan.mouseX
	self.initMouseY = chan.mouseY
	# Where are we in the channel?
        if ((abs(self.initMouseX) < 0.9) & (abs(self.initMouseY) < 0.9)):
            # Mouse is in central region
            # First compute a hit point based on current mouse position
            if(self.direct.iRay.pick(render, chan.mouseX, chan.mouseY)):
                # Find hit point in camera's space
                self.coa = self.direct.iRay.hitPt(0)
                # Handle case of bad coa point (too close or too far)
                self.coaDist = Vec3(self.coa - self.zeroPoint).length()
                if ((self.coaDist < (1.1 * self.defChan.near)) |
                    (self.coaDist > self.defChan.far)):
                    # Put it out in front of the camera
                    self.coa.set(0,-10,0)
                    self.coaDist = 10
            else:
                # If no intersection point:
                # Put coa out in front of the camera
                self.coa.set(0,-10,0)
                self.coaDist = 10

            # Now spawn task to determine mouse fly mode
            self.determineMouseFlyMode()
        else:
            # Mouse is in outer frame, spawn mouseRotateTask
            self.spawnMouseRotateTask()

    def mouseFlyStop(self):
	taskMgr.removeTasksNamed('determineMouseFlyMode')
	taskMgr.removeTasksNamed('manipulateCamera')

    def determineMouseFlyMode(self):
        if (self.direct.fShift):
            # If shift key is pressed:
            self.spawnHPPan()
        else:
            # Otherwise, determine mouse fly mode
            t = Task.Task(self.determineMouseFlyModeTask)
            taskMgr.spawnTaskNamed(t, 'determineMouseFlyMode')

    def determineMouseFlyModeTask(self, state):
        deltaX = self.defChan.mouseX - self.initMouseX
        deltaY = self.defChan.mouseY - self.initMouseY
        if ((abs(deltaX) < 0.1) & (abs(deltaY) < 0.1)):
            return Task.cont
        else:
            if (abs(deltaY) > 0.1):
                self.spawnHPanYZoom()
            else:
                self.spawnXZTranslate()
            return Task.done

    def homeCam(self, chan):
        chan.camera.setMat(Mat4.identMat())

    def uprightCam(self, chan):
	taskMgr.removeTasksNamed('manipulateCamera')
        currH = chan.camera.getH()
	chan.camera.lerpHpr(currH, 0, 0,
                            CAM_MOVE_DURATION,
                            other = render,
                            blendType = 'easeInOut',
                            task = 'manipulateCamera')

    def centerCam(self, chan):
        # Chan is a display region context
	self.centerCamIn(chan, 1.0)
        
    def centerCamNow(self, chan):
        self.centerCamIn(chan, 0.)

    def centerCamIn(self, chan,t):
        # Chan is a display region context
	taskMgr.removeTasksNamed('manipulateCamera')
	widgetToCam = self.direct.widget.getPos( chan.camera )
	dist = Vec3(widgetToCam - self.zeroPoint).length()
	scaledCenterVec = self.centerVec * dist
	delta = widgetToCam - scaledCenterVec
	self.relNodePath.setPosHpr(chan.camera, Point3(0), Point3(0))
	chan.camera.lerpPos(self.relNodePath,
                            Point3(delta),
                            CAM_MOVE_DURATION,
                            blendType = 'easeInOut',
                            task = 'manipulateCamera')

    def zoomCam(self, chan, zoomFactor, t):
	taskMgr.removeTasksNamed('manipulateCamera')
	# Find a point zoom factor times the current separation
        # of the widget and cam
	zoomPtToCam = self.direct.widget.getPos(chan.camera) * zoomFactor
	# Put a target nodePath there
	self.relNodePath.setPos(chan.camera, zoomPtToCam)
	# Move to that point
	chan.camera.lerpPos(self.zeroPoint,
                            CAM_MOVE_DURATION,
                            other = self.relNodePath,
                            blendType = 'easeInOut',
                            task = 'manipulateCamera')
        
    def SpawnMoveToView(self, chan, view):
        # Kill any existing tasks
	taskMgr.removeTasksNamed('manipulateCamera')
        # Calc hprOffset
	hprOffset = VBase3()
        if view == 1:
            hprOffset.set(180., 0., 0.)
        elif view == 2:
            hprOffset.set(0., 0., 0.)
        elif view == 3:
            hprOffset.set(90., 0., 0.)
        elif view == 4:
            hprOffset.set(-90., 0., 0.)
        elif view == 5:
            hprOffset.set(0., -90., 0.)
        elif view == 6:
            hprOffset.set(0., 90., 0.)
        elif view == 7:
            hprOffset.set(135., -35.264, 0.)
        # Position target
	self.relNodePath.setPosHpr(self.direct.widget,
                                   self.zeroBaseVec,
                                   hprOffset)
	# Scale center vec by current distance to target
	offsetDistance = Vec3(chan.camera.getPos(self.relNodePath) - 
                              self.zeroPoint).length()
	scaledCenterVec = self.centerVec * (-1.0 * offsetDistance)

   	# Now put the relNodePath at that point
	self.relNodePath.setPosHpr(self.relNodePath,
                                   scaledCenterVec,
                                   self.zeroBaseVec)

	# Start off with best view if change is to new view
        if (view != self.lastView):
            self.orthoViewRoll = 0.0
        self.lastView = view
	chan.camera.lerpPosHpr(self.zeroPoint,
                               VBase3(0,0,self.orthoViewRoll),
                               CAM_MOVE_DURATION,
                               other = self.relNodePath,
                               blendType = 'easeInOut',
                               task = 'manipulateCamera')
        
        # Try another roll next time
        self.orthoViewRoll = (self.orthoViewRoll + 90.0) % 360.0

    def swingCamAboutWidget(self, chan, degrees, t):
        # Remove existing camera manipulation task
	taskMgr.removeTasksNamed('manipulateCamera')
	
	# Coincident with widget
	self.relNodePath.setPos(self.direct.widget, self.zeroPoint)
	# But aligned with render space
	self.relNodePath.setHpr(self.zeroPoint)

	parent = self.defChan.camera.getParent
	self.defChan.camera.wrtReparentTo(self.relNodePath)

	self.relNodePath.lerpHpr(VBase3(degrees,0,0),
                                 CAM_MOVE_DURATION,
                                 blendType = 'easeInOut',
                                 task = 'manipulateCamera')
        # TODO: Convert this to an upon death
        reparentTask = Task.Task(self.reparentCam)
        reparentTask.parent = parent
        reparentLater = Task.doLater(CAM_MOVE_DURATION,
                                     reparentTask,
                                     'manipulateCamera')
        taskMgr.spawnTaskNamed(reparentLater, 'manipulateCamera')

    def reparentCam(self, state):
        self.defChan.camera.wrtReparentTo(state.parent)
        return Task.done

    def spawnHPanYZoom(self):
        # Negate vec to give it the correct sense for mouse motion below
	# targetVector = self.coa * -1
        targetVector = self.coa
        print self.coa[0], self.coa[1], self.coa[2]
        t = Task.Task(self.HPanYZoomTask)
        t.targetVector = targetVector
        taskMgr.spawnTaskNamed(t, 'manipulateCamera')

    def HPanYZoomTask(self,state):
        targetVector = state.targetVector
        distToMove = targetVector * self.defChan.mouseDeltaY
        self.defChan.camera.setPosHpr(self.defChan.camera,
                                      distToMove[0],
                                      distToMove[1],
                                      distToMove[2],
                                      (0.5 * self.defChan.mouseDeltaX *
                                       self.defChan.fovH),
                                      0.0, 0.0)
        return Task.cont


    def spawnXZTranslateOrHPPan(self):
        t = Task.Task(self.XZTranslateOrHPPanTask)
        t.scaleFactor = (self.coaDist / self.defChan.near)
        taskMgr.spawnTaskNamed(t, 'manipulateCamera')

    def XZTranslateOrHPPanTask(self, state):
        if self.direct.fShift:
            self.defChan.camera.setHpr(self.defChan.camera,
                                       (0.5 * self.defChan.mouseDeltaX *
                                        self.defChan.fovH),
                                       (-0.5 * self.defChan.mouseDeltaY *
                                        self.defChan.fovV),
                                       0.0)
        else:
            self.defChan.camera.setPos(self.defChan.camera,
                                       (-0.5 * self.defChan.mouseDeltaX *
                                        self.defChan.nearWidth *
                                        state.scaleFactor),
                                       0.0,
                                       (-0.5 * self.defChan.mouseDeltaY *
                                        self.defChan.nearHeight *
                                        state.scaleFactor))
        return Task.cont

    def spawnXZTranslate(self):
        t = Task.Task(self.XZTranslateTask)
        t.scaleFactor = (self.coaDist / self.defChan.near)
        taskMgr.spawnTaskNamed(t, 'manipulateCamera')

    def XZTranslateTask(self,state):
        self.defChan.camera.setPos(self.defChan.camera,
                                   (-0.5 * self.defChan.mouseDeltaX *
                                    self.defChan.nearWidth *
                                    state.scaleFactor),
                                   0.0,
                                   (-0.5 * self.defChan.mouseDeltaY *
                                    self.defChan.nearHeight *
                                    state.scaleFactor))
        return Task.cont

    def spawnMouseRotateTask(self):
	self.relNodePath.setPos(self.coa)
	self.relNodePath.setHpr(self.defChan.camera, self.zeroPoint)
        t = Task.Task(self.mouseRotateTask)
	t.wrtMat = self.defChan.camera.getMat( self.relNodePath )
        taskMgr.spawnTaskNamed(t, 'manipulateCamera')

    def mouseRotateTask(self, state):
        wrtMat = state.wrtMat
        self.relNodePath.setHpr(self.relNodePath,
                                (-0.5 * self.defChan.mouseDeltaX * 180.0),
                                (0.5 * self.defChan.mouseDeltaY * 180.0),
                                0.0)
        self.defChan.camera.setMat(self.relNodePath, wrtMat)
        return Task.cont

    def spawnHPPan(self):
        t = Task.Task(self.HPPanTask)
        taskMgr.spawnTaskNamed(t, 'manipulateCamera')

    def HPPanTask(self, state):
        self.defChan.camera.setHpr(self.defChan.camera,
                                   (0.5 * self.defChan.mouseDeltaX *
                                    self.defChan.fovH),
                                   (-0.5 * self.defChan.mouseDeltaY *
                                    self.defChan.fovV),
                                   0.0)
        return Task.cont

    def enableMouseFly(self):
	self.enableMouseInteraction()
	self.enableHotKeys()

    def enableMouseInteraction(self):
	# disable C++ fly interface
	base.disableMouse()
	# Accept middle mouse events
	self.accept('mouse2', self.mouseFlyStart, [self.defChan])
	self.accept('mouse2-up', self.mouseFlyStop)

    def enableHotKeys(self):
        t = CAM_MOVE_DURATION
	self.accept('u', self.uprightCam, [self.defChan])
	self.accept('c', self.centerCamIn, [self.defChan, 0.5])
	self.accept('h', self.homeCam, [self.defChan])
        for i in range(1,8):
            self.accept(`i`, self.SpawnMoveToView, [self.defChan, i])
	self.accept('9', self.swingCamAboutWidget, [self.defChan, -90.0, t])
	self.accept('0', self.swingCamAboutWidget, [self.defChan,  90.0, t])
	self.accept('8', self.removeManipulateCameraTask)
	self.accept('=', self.zoomCam, [self.defChan, 0.5, t])
	self.accept('+', self.zoomCam, [self.defChan, 0.5, t])
	self.accept('-', self.zoomCam, [self.defChan, -2.0, t])
	self.accept('_', self.zoomCam, [self.defChan, -2.0, t])

    def disableMouseFly(self):
	# Accept middle mouse events
	self.ignore('mouse2')
	self.ignore('mouse2-up')
	self.ignore('u')
	self.ignore('c')
	self.ignore('h')
        for i in range(0,10):
            self.ignore(`i`)
	self.ignore('=')
	self.ignore('+')
	self.ignore('-')
	self.ignore('=')

    def removeManipulateCameraTask(self):
        taskMgr.removeTasksNamed('manipulateCamera')



