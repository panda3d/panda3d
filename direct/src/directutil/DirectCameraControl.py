from PandaObject import *

CAM_MOVE_DURATION = 1.0

class DirectCameraControl(PandaObject):
    def __init__(self, direct):
        # Create the grid
        self.direct = direct
        self.defChan = direct.chanCenter
        self.camera = self.defChan.camera
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
            if (self.direct.fShift):
                # If shift key is pressed, just perform horiz and vert pan:
                self.spawnHPPan()
            else:
                # Otherwise, check for a hit point based on current mouse position
                # And then spawn task to determine mouse mode
                numEntries = self.direct.iRay.pick(render,chan.mouseX,chan.mouseY)
                if(numEntries):
                    # Start off with first point
                    minPt = 0
                    # Find hit point in camera's space
                    self.coa = self.direct.iRay.camToHitPt(minPt)
                    self.coaDist = Vec3(self.coa - self.zeroPoint).length()
                    # Check other intersection points, sorting them
                    # TBD: Use TBS C++ function to do this
                    if numEntries > 1:
                        for i in range(1,numEntries):
                            hitPt = self.direct.iRay.camToHitPt(i)
                            dist = Vec3(hitPt - self.zeroPoint).length()
                            if (dist < self.coaDist):
                                self.coaDist = dist
                                self.coa = hitPt
                                minPt = i

                    # Handle case of bad coa point (too close or too far)
                    if ((self.coaDist < (1.1 * self.defChan.near)) |
                        (self.coaDist > self.defChan.far)):
                        # Put it out in front of the camera
                        self.coa.set(0,10,0)
                        self.coaDist = 10
                else:
                    # If no intersection point:
                    # Put coa out in front of the camera
                    self.coa.set(0,10,0)
                    self.coaDist = 10

                # Place the marker in render space
                self.coaMarker.setPos(self.camera,self.coa)
                # Record this point for later use
                self.coaMarkerPos = self.coaMarker.getPos()
                # Now spawn task to determine mouse fly mode
                self.determineMouseFlyMode()
            # END MOUSE IN CENTRAL REGION
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

        # Store this view for next time
        # Reset orthoViewRoll if you change views
        if view != self.lastView:
            self.orthoViewRoll = 0.0
            
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

	parent = self.defChan.camera.getParent()
	self.defChan.camera.wrtReparentTo(self.relNodePath)

	manipTask = self.relNodePath.lerpHpr(VBase3(degrees,0,0),
                                             CAM_MOVE_DURATION,
                                             blendType = 'easeInOut',
                                             task = 'manipulateCamera')
        # Upon death, reparent Cam to parent
        manipTask.parent = parent
        manipTask.uponDeath = self.reparentCam

    def reparentCam(self, state):
        self.defChan.camera.wrtReparentTo(state.parent)

    def spawnHPanYZoom(self):
        # Negate vec to give it the correct sense for mouse motion below
	# targetVector = self.coa * -1
        targetVector = self.coa * -1
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
	self.relNodePath.setPos(render, self.coaMarkerPos)
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
        self.coaMarker.reparentTo(render)

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
        for i in range(1,9):
            self.accept(`i`, self.SpawnMoveToView, [self.defChan, i])
	self.accept('9', self.swingCamAboutWidget, [self.defChan, -90.0, t])
	self.accept('0', self.swingCamAboutWidget, [self.defChan,  90.0, t])
	self.accept('`', self.removeManipulateCameraTask)
	self.accept('=', self.zoomCam, [self.defChan, 0.5, t])
	self.accept('+', self.zoomCam, [self.defChan, 0.5, t])
	self.accept('-', self.zoomCam, [self.defChan, -2.0, t])
	self.accept('_', self.zoomCam, [self.defChan, -2.0, t])

    def disableMouseFly(self):
        # Hide the marker
        self.coaMarker.reparentTo(hidden)
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
	self.ignore('_')
        self.ignore('`')

    def removeManipulateCameraTask(self):
        taskMgr.removeTasksNamed('manipulateCamera')

