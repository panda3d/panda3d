from PandaObject import *
from DirectGeometry import useDirectRenderStyle

CAM_MOVE_DURATION = 1.0
COA_MARKER_SF = 0.0075
Y_AXIS = Vec3(0,1,0)

class DirectCameraControl(PandaObject):
    def __init__(self):
        # Create the grid
	self.orthoViewRoll = 0.0
	self.lastView = 0
        self.coa = Point3(0,100,0)
        self.coaDist = 100
        self.coaMarker = loader.loadModel('models/misc/sphere')
        self.coaMarker.setName('DirectCameraCOAMarker')
        self.coaMarker.setColor(1,0,0)
        self.coaMarker.setPos(0,0,0)
        useDirectRenderStyle(self.coaMarker)
        self.coaMarkerPos = Point3(0)
	self.camManipRef = direct.group.attachNewNode('camManipRef')
        self.zeroBaseVec = VBase3(0)
        self.zeroVector = Vec3(0)
        self.centerVec = Vec3(0, 1, 0)
        self.zeroPoint = Point3(0)
        t = CAM_MOVE_DURATION
        self.actionEvents = [
            ['handleMouse2', self.mouseFlyStart],
            ['handleMouse2Up', self.mouseFlyStop],
            ['u', self.uprightCam],
            ['c', self.centerCamIn, 0.5],
            ['h', self.homeCam],
            ['f', self.fitOnWidget],
            [`1`, self.SpawnMoveToView, 1],
            [`2`, self.SpawnMoveToView, 2],
            [`3`, self.SpawnMoveToView, 3],
            [`4`, self.SpawnMoveToView, 4],
            [`5`, self.SpawnMoveToView, 5],
            [`6`, self.SpawnMoveToView, 6],
            [`7`, self.SpawnMoveToView, 7],
            [`8`, self.SpawnMoveToView, 8],
            ['9', self.swingCamAboutWidget, -90.0, t],
            ['0', self.swingCamAboutWidget,  90.0, t],
            ['`', self.removeManipulateCameraTask],
            ['=', self.zoomCam, 0.5, t],
            ['+', self.zoomCam, 0.5, t],
            ['-', self.zoomCam, -2.0, t],
            ['_', self.zoomCam, -2.0, t],
            ]

    def mouseFlyStart(self):
	# Record starting mouse positions
	self.initMouseX = direct.dr.mouseX
	self.initMouseY = direct.dr.mouseY
        # Record undo point
        direct.pushUndo([direct.camera])
	# Where are we in the display region?
        if ((abs(self.initMouseX) < 0.9) & (abs(self.initMouseY) < 0.9)):
            # MOUSE IS IN CENTRAL REGION
            # Hide the marker for this kind of motion
            self.coaMarker.hide()
            # See if the shift key is pressed
            if (direct.fShift):
                # If shift key is pressed, just perform horiz and vert pan:
                self.spawnHPPan()
            else:
                # Otherwise, check for a hit point based on
                # current mouse position
                # And then spawn task to determine mouse mode
                numEntries = direct.iRay.pickGeom(
                    render,direct.dr.mouseX,direct.dr.mouseY)
                # Filter out hidden nodes from entry list
                indexList = []
                for i in range(0,numEntries):
                    entry = direct.iRay.cq.getEntry(i)
                    node = entry.getIntoNode()
                    if node.isHidden():
                        pass
                    else:
                        # Not one of the widgets, use it
                        indexList.append(i)
                coa = Point3(0)
                if(indexList):
                    # Start off with first point
                    minPt = indexList[0]
                    # Find hit point in camera's space
                    hitPt = direct.iRay.camToHitPt(minPt)
                    coa.set(hitPt[0],hitPt[1],hitPt[2])
                    coaDist = Vec3(coa - self.zeroPoint).length()
                    # Check other intersection points, sorting them
                    # TBD: Use TBS C++ function to do this
                    if len(indexList) > 1:
                        for i in range(1,len(indexList)):
                            entryNum = indexList[i]
                            hitPt = direct.iRay.camToHitPt(entryNum)
                            dist = Vec3(hitPt - self.zeroPoint).length()
                            if (dist < coaDist):
                                coaDist = dist
                                coa.set(hitPt[0],hitPt[1],hitPt[2])
                                minPt = i
                    # Handle case of bad coa point (too close or too far)
                    if ((coaDist < (1.1 * direct.dr.near)) |
                        (coaDist > direct.dr.far)):
                        # Just use existing point
                        coa.assign(self.coaMarker.getPos(direct.camera))
                        coaDist = Vec3(coa - self.zeroPoint).length()
                        if coaDist < (1.1 * direct.dr.near):
                            coa.set(0,100,0)
                            coaDist = 100
                else:
                    # If no intersection point:
                    # Use existing point
                    coa.assign(self.coaMarker.getPos(direct.camera))
                    coaDist = Vec3(coa - self.zeroPoint).length()
                    if coaDist < (1.1 * direct.dr.near):
                        coa.set(0,100,0)
                        coaDist = 100
                # Update coa and marker
                self.updateCoa(coa, coaDist)
                # Now spawn task to determine mouse fly mode
                self.determineMouseFlyMode()
            # END MOUSE IN CENTRAL REGION
        else:
            # Mouse is in outer frame, spawn mouseRotateTask
            self.spawnMouseRotateTask()

    def mouseFlyStop(self):
	taskMgr.removeTasksNamed('determineMouseFlyMode')
	taskMgr.removeTasksNamed('manipulateCamera')
        # Show the marker
        self.coaMarker.show()
        # Resize it
        self.updateCoaMarkerSize()

    def determineMouseFlyMode(self):
        # Otherwise, determine mouse fly mode
        t = Task.Task(self.determineMouseFlyModeTask)
        taskMgr.spawnTaskNamed(t, 'determineMouseFlyMode')

    def determineMouseFlyModeTask(self, state):
        deltaX = direct.dr.mouseX - self.initMouseX
        deltaY = direct.dr.mouseY - self.initMouseY
        if ((abs(deltaX) < 0.1) & (abs(deltaY) < 0.1)):
            return Task.cont
        else:
            if (abs(deltaY) > 0.1):
                self.spawnHPanYZoom()
            else:
                self.spawnXZTranslate()
            return Task.done

    def updateCoa(self, cam2point, coaDist = None):
        self.coa.set(cam2point[0], cam2point[1], cam2point[2])
        if coaDist:
            self.coaDist = coaDist
        else:
            self.coaDist = Vec3(self.coa - self.zeroPoint).length()
        # Place the marker in render space
        self.coaMarker.setPos(direct.camera,self.coa)
        # Resize it
        self.updateCoaMarkerSize(coaDist)
        # Record marker pos in render space
        self.coaMarkerPos.assign(self.coaMarker.getPos())

    def updateCoaMarkerSize(self, coaDist = None):
        if not coaDist:
            coaDist = Vec3(self.coaMarker.getPos( direct.camera )).length()
        self.coaMarker.setScale(COA_MARKER_SF * coaDist *
                                math.tan(deg2Rad(direct.dr.fovV)))

    def homeCam(self):
        # Record undo point
        direct.pushUndo([direct.camera])
        direct.camera.setMat(Mat4.identMat())

    def uprightCam(self):
	taskMgr.removeTasksNamed('manipulateCamera')
        currH = direct.camera.getH()
        # Record undo point
        direct.pushUndo([direct.camera])
	direct.camera.lerpHpr(currH, 0, 0,
                              CAM_MOVE_DURATION,
                              other = render,
                              blendType = 'easeInOut',
                              task = 'manipulateCamera')

    def centerCam(self):
	self.centerCamIn(1.0)
        
    def centerCamNow(self):
        self.centerCamIn(0.)

    def centerCamIn(self, t):
	taskMgr.removeTasksNamed('manipulateCamera')
        # Record undo point
        direct.pushUndo([direct.camera])
        # Determine marker location
        markerToCam = self.coaMarker.getPos( direct.camera )
	dist = Vec3(markerToCam - self.zeroPoint).length()
	scaledCenterVec = self.centerVec * dist
	delta = markerToCam - scaledCenterVec
	self.camManipRef.setPosHpr(direct.camera, Point3(0), Point3(0))
	direct.camera.lerpPos(Point3(delta),
                            CAM_MOVE_DURATION,
                            other = self.camManipRef,
                            blendType = 'easeInOut',
                            task = 'manipulateCamera')

    def zoomCam(self, zoomFactor, t):
	taskMgr.removeTasksNamed('manipulateCamera')
        # Record undo point
        direct.pushUndo([direct.camera])
	# Find a point zoom factor times the current separation
        # of the widget and cam
        zoomPtToCam = self.coaMarker.getPos(direct.camera) * zoomFactor
	# Put a target nodePath there
	self.camManipRef.setPos(direct.camera, zoomPtToCam)
	# Move to that point
	direct.camera.lerpPos(self.zeroPoint,
                            CAM_MOVE_DURATION,
                            other = self.camManipRef,
                            blendType = 'easeInOut',
                            task = 'manipulateCamera')
        
    def SpawnMoveToView(self, view):
        # Kill any existing tasks
        taskMgr.removeTasksNamed('manipulateCamera')
        # Record undo point
        direct.pushUndo([direct.camera])
        # Calc hprOffset
        hprOffset = VBase3()
        if view == 8:
            # Try the next roll angle
            self.orthoViewRoll = (self.orthoViewRoll + 90.0) % 360.0
            # but use the last view
            view = self.lastView
        else:
            self.orthoViewRoll = 0.0
        # Adjust offset based on specified view
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
        self.camManipRef.setPosHpr(self.coaMarker, self.zeroBaseVec,
                                   hprOffset)
        # Scale center vec by current distance to target
        offsetDistance = Vec3(direct.camera.getPos(self.camManipRef) - 
                              self.zeroPoint).length()
        scaledCenterVec = self.centerVec * (-1.0 * offsetDistance)
        # Now put the camManipRef at that point
        self.camManipRef.setPosHpr(self.camManipRef,
                                   scaledCenterVec,
                                   self.zeroBaseVec)
        # Record view for next time around
        self.lastView = view
        direct.camera.lerpPosHpr(self.zeroPoint,
                                 VBase3(0,0,self.orthoViewRoll),
                                 CAM_MOVE_DURATION,
                                 other = self.camManipRef,
                                 blendType = 'easeInOut',
                                 task = 'manipulateCamera')

        
    def swingCamAboutWidget(self, degrees, t):
        # Remove existing camera manipulation task
	taskMgr.removeTasksNamed('manipulateCamera')
	
        # Record undo point
        direct.pushUndo([direct.camera])
        
	# Coincident with widget
        self.camManipRef.setPos(self.coaMarker, self.zeroPoint)
	# But aligned with render space
	self.camManipRef.setHpr(self.zeroPoint)

	parent = direct.camera.getParent()
	direct.camera.wrtReparentTo(self.camManipRef)

	manipTask = self.camManipRef.lerpHpr(VBase3(degrees,0,0),
                                             CAM_MOVE_DURATION,
                                             blendType = 'easeInOut',
                                             task = 'manipulateCamera')
        # Upon death, reparent Cam to parent
        manipTask.parent = parent
        manipTask.uponDeath = self.reparentCam

    def reparentCam(self, state):
        direct.camera.wrtReparentTo(state.parent)

    def spawnHPanYZoom(self):
        # Kill any existing tasks
	taskMgr.removeTasksNamed('manipulateCamera')
        # hide the marker
        self.coaMarker.hide()
        # Negate vec to give it the correct sense for mouse motion below
        targetVector = self.coa * -1
        t = Task.Task(self.HPanYZoomTask)
        t.targetVector = targetVector
        taskMgr.spawnTaskNamed(t, 'manipulateCamera')

    def HPanYZoomTask(self,state):
        targetVector = state.targetVector
        # Can bring object to you by dragging across half the screen
        distToMove = targetVector * (2.0 * direct.dr.mouseDeltaY)
        direct.camera.setPosHpr(direct.camera,
                                distToMove[0],
                                distToMove[1],
                                distToMove[2],
                                (0.5 * direct.dr.mouseDeltaX *
                                 direct.dr.fovH),
                                0.0, 0.0)
        return Task.cont


    def spawnXZTranslateOrHPPan(self):
        # Kill any existing tasks
	taskMgr.removeTasksNamed('manipulateCamera')
        # Hide the marker
        self.coaMarker.hide()
        t = Task.Task(self.XZTranslateOrHPPanTask)
        t.scaleFactor = (self.coaDist / direct.dr.near)
        taskMgr.spawnTaskNamed(t, 'manipulateCamera')

    def XZTranslateOrHPPanTask(self, state):
        if direct.fShift:
            direct.camera.setHpr(direct.camera,
                               (0.5 * direct.dr.mouseDeltaX *
                                direct.dr.fovH),
                               (-0.5 * direct.dr.mouseDeltaY *
                                direct.dr.fovV),
                               0.0)
        else:
            direct.camera.setPos(direct.camera,
                               (-0.5 * direct.dr.mouseDeltaX *
                                direct.dr.nearWidth *
                                state.scaleFactor),
                               0.0,
                               (-0.5 * direct.dr.mouseDeltaY *
                                direct.dr.nearHeight *
                                state.scaleFactor))
        return Task.cont

    def spawnXZTranslate(self):
        # Kill any existing tasks
	taskMgr.removeTasksNamed('manipulateCamera')
        # Hide the marker
        self.coaMarker.hide()
        t = Task.Task(self.XZTranslateTask)
        t.scaleFactor = (self.coaDist / direct.dr.near)
        taskMgr.spawnTaskNamed(t, 'manipulateCamera')

    def XZTranslateTask(self,state):
        direct.camera.setPos(direct.camera,
                             (-0.5 * direct.dr.mouseDeltaX *
                              direct.dr.nearWidth *
                              state.scaleFactor),
                             0.0,
                             (-0.5 * direct.dr.mouseDeltaY *
                              direct.dr.nearHeight *
                              state.scaleFactor))
        return Task.cont

    def spawnMouseRotateTask(self):
        # Kill any existing tasks
	taskMgr.removeTasksNamed('manipulateCamera')
        # Set at markers position in render coordinates
	self.camManipRef.setPos(self.coaMarkerPos)
	self.camManipRef.setHpr(direct.camera, self.zeroPoint)
        t = Task.Task(self.mouseRotateTask)
	t.wrtMat = direct.camera.getMat( self.camManipRef )
        taskMgr.spawnTaskNamed(t, 'manipulateCamera')

    def mouseRotateTask(self, state):
        wrtMat = state.wrtMat
        self.camManipRef.setHpr(self.camManipRef,
                                (-0.5 * direct.dr.mouseDeltaX * 180.0),
                                (0.5 * direct.dr.mouseDeltaY * 180.0),
                                0.0)
        direct.camera.setMat(self.camManipRef, wrtMat)
        return Task.cont

    def spawnHPPan(self):
        # Kill any existing tasks
	taskMgr.removeTasksNamed('manipulateCamera')
        # Hide the marker
        self.coaMarker.hide()
        t = Task.Task(self.HPPanTask)
        taskMgr.spawnTaskNamed(t, 'manipulateCamera')

    def HPPanTask(self, state):
        direct.camera.setHpr(direct.camera,
                             (0.5 * direct.dr.mouseDeltaX *
                              direct.dr.fovH),
                             (-0.5 * direct.dr.mouseDeltaY *
                              direct.dr.fovV),
                             0.0)
        return Task.cont

    def fitOnWidget(self):
        # Fit the node on the screen
        
        # stop any ongoing tasks
        taskMgr.removeTasksNamed('manipulateCamera')

        # How big is the node?
        nodeScale = direct.widget.scalingNode.getScale(render)
        maxScale = max(nodeScale[0],nodeScale[1],nodeScale[2])
        maxDim = min(direct.dr.nearWidth, direct.dr.nearHeight)

        # At what distance does the object fill 30% of the screen?
        # Assuming radius of 1 on widget
        camY = direct.dr.near * (2.0 * maxScale)/(0.3 * maxDim)
    
        # What is the vector through the center of the screen?
        centerVec = Y_AXIS * camY
    
        # Where is the node relative to the viewpoint
        vWidget2Camera = direct.widget.getPos(direct.camera)
    
        # How far do you move the camera to be this distance from the node?
        deltaMove = vWidget2Camera - centerVec
    
        # Move a target there
        self.camManipRef.setPos(direct.camera, deltaMove)

	parent = direct.camera.getParent()
	direct.camera.wrtReparentTo(self.camManipRef)
	fitTask = direct.camera.lerpPos(Point3(0,0,0),
                                      CAM_MOVE_DURATION,
                                      blendType = 'easeInOut',
                                      task = 'manipulateCamera')
        # Upon death, reparent Cam to parent
        fitTask.parent = parent
        fitTask.uponDeath = self.reparentCam                                

    def enableMouseFly(self):
	# disable C++ fly interface
	base.disableMouse()
        # Enable events
        for event in self.actionEvents:
            self.accept(event[0], event[1], extraArgs = event[2:])
        # Show marker
        self.coaMarker.reparentTo(direct.group)

    def disableMouseFly(self):
        # Hide the marker
        self.coaMarker.reparentTo(hidden)
	# Ignore events
        for event in self.actionEvents:
            self.ignore(event[0])

    def removeManipulateCameraTask(self):
        taskMgr.removeTasksNamed('manipulateCamera')

