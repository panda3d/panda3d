from PandaObject import *

CAM_MOVE_DURATION = 1.0

class DirectCameraControl(PandaObject):
    def __init__(self, direct):
        # Create the grid
        self.direct = direct
        self.chan = direct.chan
        self.camera = self.chan.camera
	self.orthoViewRoll = 0.0
	self.lastView = 0
        self.coa = Point3(0)
        self.coaMarker = loader.loadModel('misc/sphere')
        self.coaMarker.setColor(1,0,0)
        self.coaMarkerPos = Point3(0)
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
            # MOUSE IS IN CENTRAL REGION
            # Hide the marker for this kind of motion
            self.coaMarker.hide()
            # See if the shift key is pressed
            if (self.direct.fShift):
                # If shift key is pressed, just perform horiz and vert pan:
                self.spawnHPPan()
            else:
                # Otherwise, check for a hit point based on current mouse position
                # And then spawn task to determine mouse mode
                numEntries = self.direct.iRay.pickGeom(
                    render,chan.mouseX,chan.mouseY)
                coa = Point3(0)
                if(numEntries):
                    # Start off with first point
                    minPt = 0
                    # Find hit point in camera's space
                    hitPt = self.direct.iRay.camToHitPt(minPt)
                    coa.set(hitPt[0],hitPt[1],hitPt[2])
                    coaDist = Vec3(coa - self.zeroPoint).length()
                    # Check other intersection points, sorting them
                    # TBD: Use TBS C++ function to do this
                    if numEntries > 1:
                        for i in range(1,numEntries):
                            hitPt = self.direct.iRay.camToHitPt(i)
                            dist = Vec3(hitPt - self.zeroPoint).length()
                            if (dist < coaDist):
                                coaDist = dist
                                coa.set(hitPt[0],hitPt[1],hitPt[2])
                                minPt = i

                    # Handle case of bad coa point (too close or too far)
                    if ((coaDist < (1.1 * self.chan.near)) |
                        (coaDist > self.chan.far)):
                        # Put it out in front of the camera
                        coa.set(0,100,0)
                        coaDist = 100
                else:
                    # If no intersection point:
                    # Put coa out in front of the camera
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
        deltaX = self.chan.mouseX - self.initMouseX
        deltaY = self.chan.mouseY - self.initMouseY
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
        self.coaMarker.setPos(self.camera,self.coa)
        # Resize it
        self.updateCoaMarkerSize(coaDist)
        # Record marker pos in render space
        self.coaMarkerPos.assign(self.coaMarker.getPos())

    def updateCoaMarkerSize(self, coaDist = None):
        if not coaDist:
            coaDist = Vec3(self.coaMarker.getPos( self.chan.camera )).length()
        self.coaMarker.setScale(0.01 * coaDist *
                                math.tan(deg2Rad(self.chan.fovV)))

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
        markerToCam = self.coaMarker.getPos( chan.camera )
	dist = Vec3(markerToCam - self.zeroPoint).length()
	scaledCenterVec = self.centerVec * dist
	delta = markerToCam - scaledCenterVec
	self.relNodePath.setPosHpr(chan.camera, Point3(0), Point3(0))
	chan.camera.lerpPos(Point3(delta),
                            CAM_MOVE_DURATION,
                            other = self.relNodePath,
                            blendType = 'easeInOut',
                            task = 'manipulateCamera')

    def zoomCam(self, chan, zoomFactor, t):
	taskMgr.removeTasksNamed('manipulateCamera')
	# Find a point zoom factor times the current separation
        # of the widget and cam
        zoomPtToCam = self.coaMarker.getPos(chan.camera) * zoomFactor
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
        self.relNodePath.setPosHpr(self.coaMarker, self.zeroBaseVec,
                                   hprOffset)
        # Scale center vec by current distance to target
        offsetDistance = Vec3(chan.camera.getPos(self.relNodePath) - 
                              self.zeroPoint).length()
        scaledCenterVec = self.centerVec * (-1.0 * offsetDistance)
        # Now put the relNodePath at that point
        self.relNodePath.setPosHpr(self.relNodePath,
                                   scaledCenterVec,
                                   self.zeroBaseVec)
        # Record view for next time around
        self.lastView = view
        chan.camera.lerpPosHpr(self.zeroPoint,
                               VBase3(0,0,self.orthoViewRoll),
                               CAM_MOVE_DURATION,
                               other = self.relNodePath,
                               blendType = 'easeInOut',
                               task = 'manipulateCamera')

        
    def swingCamAboutWidget(self, chan, degrees, t):
        # Remove existing camera manipulation task
	taskMgr.removeTasksNamed('manipulateCamera')
	
	# Coincident with widget
        self.relNodePath.setPos(self.coaMarker, self.zeroPoint)
	# But aligned with render space
	self.relNodePath.setHpr(self.zeroPoint)

	parent = self.camera.getParent()
	self.camera.wrtReparentTo(self.relNodePath)

	manipTask = self.relNodePath.lerpHpr(VBase3(degrees,0,0),
                                             CAM_MOVE_DURATION,
                                             blendType = 'easeInOut',
                                             task = 'manipulateCamera')
        # Upon death, reparent Cam to parent
        manipTask.parent = parent
        manipTask.uponDeath = self.reparentCam

    def reparentCam(self, state):
        self.camera.wrtReparentTo(state.parent)

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
        distToMove = targetVector * self.chan.mouseDeltaY
        self.camera.setPosHpr(self.camera,
                              distToMove[0],
                              distToMove[1],
                              distToMove[2],
                              (0.5 * self.chan.mouseDeltaX *
                               self.chan.fovH),
                              0.0, 0.0)
        return Task.cont


    def spawnXZTranslateOrHPPan(self):
        # Kill any existing tasks
	taskMgr.removeTasksNamed('manipulateCamera')
        # Hide the marker
        self.coaMarker.hide()
        t = Task.Task(self.XZTranslateOrHPPanTask)
        t.scaleFactor = (self.coaDist / self.chan.near)
        taskMgr.spawnTaskNamed(t, 'manipulateCamera')

    def XZTranslateOrHPPanTask(self, state):
        if self.direct.fShift:
            self.camera.setHpr(self.camera,
                               (0.5 * self.chan.mouseDeltaX *
                                self.chan.fovH),
                               (-0.5 * self.chan.mouseDeltaY *
                                self.chan.fovV),
                               0.0)
        else:
            self.camera.setPos(self.camera,
                               (-0.5 * self.chan.mouseDeltaX *
                                self.chan.nearWidth *
                                state.scaleFactor),
                               0.0,
                               (-0.5 * self.chan.mouseDeltaY *
                                self.chan.nearHeight *
                                state.scaleFactor))
        return Task.cont

    def spawnXZTranslate(self):
        # Kill any existing tasks
	taskMgr.removeTasksNamed('manipulateCamera')
        # Hide the marker
        self.coaMarker.hide()
        t = Task.Task(self.XZTranslateTask)
        t.scaleFactor = (self.coaDist / self.chan.near)
        taskMgr.spawnTaskNamed(t, 'manipulateCamera')

    def XZTranslateTask(self,state):
        self.camera.setPos(self.camera,
                           (-0.5 * self.chan.mouseDeltaX *
                            self.chan.nearWidth *
                            state.scaleFactor),
                           0.0,
                           (-0.5 * self.chan.mouseDeltaY *
                            self.chan.nearHeight *
                            state.scaleFactor))
        return Task.cont

    def spawnMouseRotateTask(self):
        # Kill any existing tasks
	taskMgr.removeTasksNamed('manipulateCamera')
        # Set at markers position in render coordinates
	self.relNodePath.setPos(self.coaMarkerPos)
	self.relNodePath.setHpr(self.camera, self.zeroPoint)
        t = Task.Task(self.mouseRotateTask)
	t.wrtMat = self.camera.getMat( self.relNodePath )
        taskMgr.spawnTaskNamed(t, 'manipulateCamera')

    def mouseRotateTask(self, state):
        wrtMat = state.wrtMat
        self.relNodePath.setHpr(self.relNodePath,
                                (-0.5 * self.chan.mouseDeltaX * 180.0),
                                (0.5 * self.chan.mouseDeltaY * 180.0),
                                0.0)
        self.camera.setMat(self.relNodePath, wrtMat)
        return Task.cont

    def spawnHPPan(self):
        # Kill any existing tasks
	taskMgr.removeTasksNamed('manipulateCamera')
        # Hide the marker
        self.coaMarker.hide()
        t = Task.Task(self.HPPanTask)
        taskMgr.spawnTaskNamed(t, 'manipulateCamera')

    def HPPanTask(self, state):
        self.camera.setHpr(self.camera,
                           (0.5 * self.chan.mouseDeltaX *
                            self.chan.fovH),
                           (-0.5 * self.chan.mouseDeltaY *
                            self.chan.fovV),
                           0.0)
        return Task.cont

    def enableMouseFly(self):
	self.enableMouseInteraction()
	self.enableHotKeys()
        self.coaMarker.reparentTo(render)

    def enableMouseInteraction(self):
	# disable C++ fly interface
	base.disableMouse()
	# Accept middle mouse events
	self.accept('handleMouse2', self.mouseFlyStart, [self.chan])
	self.accept('handleMouse2Up', self.mouseFlyStop)

    def enableHotKeys(self):
        t = CAM_MOVE_DURATION
	self.accept('u', self.uprightCam, [self.chan])
	self.accept('c', self.centerCamIn, [self.chan, 0.5])
	self.accept('h', self.homeCam, [self.chan])
        for i in range(1,9):
            self.accept(`i`, self.SpawnMoveToView, [self.chan, i])
	self.accept('9', self.swingCamAboutWidget, [self.chan, -90.0, t])
	self.accept('0', self.swingCamAboutWidget, [self.chan,  90.0, t])
	self.accept('`', self.removeManipulateCameraTask)
	self.accept('=', self.zoomCam, [self.chan, 0.5, t])
	self.accept('+', self.zoomCam, [self.chan, 0.5, t])
	self.accept('-', self.zoomCam, [self.chan, -2.0, t])
	self.accept('_', self.zoomCam, [self.chan, -2.0, t])

    def disableMouseFly(self):
        # Hide the marker
        self.coaMarker.reparentTo(hidden)
	# Ignore middle mouse events
	self.ignore('handleMouse2')
	self.ignore('handleMouse2Up')
	self.ignore('u')
	self.ignore('c')
	self.ignore('h')
        for i in range(0,10):
            self.ignore(`i`)
	self.ignore('=')
	self.ignore('+')
	self.ignore('-')
	self.ignore('_')
        self.ignore('`')

    def removeManipulateCameraTask(self):
        taskMgr.removeTasksNamed('manipulateCamera')

