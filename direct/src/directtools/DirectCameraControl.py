from PandaObject import *
from DirectUtil import *
from DirectGeometry import *
import Task

CAM_MOVE_DURATION = 1.2
COA_MARKER_SF = 0.0075
Y_AXIS = Vec3(0,1,0)

class DirectCameraControl(PandaObject):
    def __init__(self):
        # Create the grid
        self.startT = 0.0
        self.startF = 0
        self.orthoViewRoll = 0.0
        self.lastView = 0
        self.coa = Point3(0,100,0)
        self.coaMarker = loader.loadModel('models/misc/sphere')
        self.coaMarker.setName('DirectCameraCOAMarker')
        self.coaMarker.setTransparency(1)
        self.coaMarker.setColor(1,0,0,0)
        self.coaMarker.setPos(0,100,0)
        useDirectRenderStyle(self.coaMarker)
        self.coaMarkerPos = Point3(0)
        self.fLockCOA = 0
        self.nullHitPointCount = 0
        self.cqEntries = []
        self.coaMarkerRef = direct.group.attachNewNode('coaMarkerRef')
        self.camManipRef = direct.group.attachNewNode('camManipRef')
        t = CAM_MOVE_DURATION
        self.actionEvents = [
            ['DIRECT-mouse2', self.mouseFlyStart],
            ['DIRECT-mouse2Up', self.mouseFlyStop],
            ['c', self.centerCamIn, 0.5],
            ['f', self.fitOnWidget],
            ['h', self.homeCam],
            ['shift-v', self.toggleMarkerVis],
            ['m', self.moveToFit],
            ['n', self.pickNextCOA],
            ['u', self.orbitUprightCam],
            ['shift-u', self.uprightCam],
            [`1`, self.spawnMoveToView, 1],
            [`2`, self.spawnMoveToView, 2],
            [`3`, self.spawnMoveToView, 3],
            [`4`, self.spawnMoveToView, 4],
            [`5`, self.spawnMoveToView, 5],
            [`6`, self.spawnMoveToView, 6],
            [`7`, self.spawnMoveToView, 7],
            [`8`, self.spawnMoveToView, 8],
            ['9', self.swingCamAboutWidget, -90.0, t],
            ['0', self.swingCamAboutWidget,  90.0, t],
            ['`', self.removeManipulateCameraTask],
            ['=', self.zoomCam, 0.5, t],
            ['+', self.zoomCam, 0.5, t],
            ['-', self.zoomCam, -2.0, t],
            ['_', self.zoomCam, -2.0, t],
            ]

    def toggleMarkerVis(self):
        if direct.cameraControl.coaMarker.isHidden():
            direct.cameraControl.coaMarker.show()
        else:
            direct.cameraControl.coaMarker.hide()

    def mouseFlyStart(self, modifiers):
        # Record undo point
        direct.pushUndo([direct.camera])
        # Where are we in the display region?
        if ((abs(direct.dr.mouseX) < 0.9) and (abs(direct.dr.mouseY) < 0.9)):
            # MOUSE IS IN CENTRAL REGION
            # Hide the marker for this kind of motion
            self.coaMarker.hide()
            # Record time of start of mouse interaction
            self.startT= globalClock.getFrameTime()
            self.startF = globalClock.getFrameCount()
            # Start manipulation
            self.spawnXZTranslateOrHPanYZoom()
            # END MOUSE IN CENTRAL REGION
        else:
            if ((abs(direct.dr.mouseX) > 0.9) and
                (abs(direct.dr.mouseY) > 0.9)):
                # Mouse is in corners, spawn roll task
                self.spawnMouseRollTask()
            else:
                # Mouse is in outer frame, spawn mouseRotateTask
                self.spawnMouseRotateTask()

    def mouseFlyStop(self):
        taskMgr.remove('manipulateCamera')
        stopT = globalClock.getFrameTime()
        deltaT = stopT - self.startT
        stopF = globalClock.getFrameCount()
        deltaF = stopF - self.startF
        if (deltaT <= 0.25) or (deltaF <= 1):
            # Check for a hit point based on
            # current mouse position
            # Allow intersection with unpickable objects
            # And then spawn task to determine mouse mode
            node, hitPt, hitPtDist = direct.iRay.pickGeom(
                fIntersectUnpickable = 1,
                fIgnoreCamera = 1 - base.getControl())
            self.computeCOA(node, hitPt, hitPtDist)
            # Record reference point
            self.coaMarkerRef.iPosHprScale(direct.iRay.collisionRef)
            # Record entries
            self.cqEntries = []
            for i in range(direct.iRay.cq.getNumEntries()):
                self.cqEntries.append(direct.iRay.cq.getEntry(i))
        # Show the marker
        self.coaMarker.show()
        # Resize it
        self.updateCoaMarkerSize()

    def spawnXZTranslateOrHPanYZoom(self):
        # Kill any existing tasks
        taskMgr.remove('manipulateCamera')
        # Spawn the new task
        t = Task.Task(self.XZTranslateOrHPanYZoomTask)
        # For HPanYZoom
        t.zoomSF = Vec3(self.coaMarker.getPos(direct.camera)).length()
        taskMgr.add(t, 'manipulateCamera')

    def spawnXZTranslateOrHPPan(self):
        # Kill any existing tasks
        taskMgr.remove('manipulateCamera')
        # Spawn new task
        taskMgr.add(self.XZTranslateOrHPPanTask,
                    'manipulateCamera')

    def spawnXZTranslate(self):
        # Kill any existing tasks
        taskMgr.remove('manipulateCamera')
        # Spawn new task
        taskMgr.add(self.XZTranslateTask, 'manipulateCamera')

    def spawnHPanYZoom(self):
        # Kill any existing tasks
        taskMgr.remove('manipulateCamera')
        # Spawn new task
        t = Task.Task(self.HPanYZoomTask)
        t.zoomSF = Vec3(self.coaMarker.getPos(direct.camera)).length()
        taskMgr.add(t, 'manipulateCamera')

    def spawnHPPan(self):
        # Kill any existing tasks
        taskMgr.remove('manipulateCamera')
        # Spawn new task
        taskMgr.add(self.HPPanTask, 'manipulateCamera')

    def XZTranslateOrHPanYZoomTask(self, state):
        if direct.fShift:
            return self.XZTranslateTask(state)
        else:
            return self.HPanYZoomTask(state)

    def XZTranslateOrHPPanTask(self, state):
        if direct.fShift:
            # Panning action
            return self.HPPanTask(state)
        else:
            # Translation action
            return self.XZTranslateTask(state)

    def XZTranslateTask(self,state):
        coaDist = Vec3(self.coaMarker.getPos(direct.camera)).length()
        xlateSF = (coaDist / direct.dr.near)
        direct.camera.setPos(direct.camera,
                             (-0.5 * direct.dr.mouseDeltaX *
                              direct.dr.nearWidth *
                              xlateSF),
                             0.0,
                             (-0.5 * direct.dr.mouseDeltaY *
                              direct.dr.nearHeight *
                              xlateSF))
        return Task.cont

    def HPanYZoomTask(self,state):
        if direct.fControl:
            moveDir = Vec3(self.coaMarker.getPos(direct.camera))
            # If marker is behind camera invert vector
            if moveDir[1] < 0.0:
                moveDir.assign(moveDir * -1)
            moveDir.normalize()
        else:
            moveDir = Vec3(Y_AXIS)
        moveDir.assign(moveDir * (-1.0 * direct.dr.mouseDeltaY *
                                        state.zoomSF))
        if direct.dr.mouseDeltaY > 0.0:
            moveDir.setY(moveDir[1] * 1.0)
        direct.camera.setPosHpr(direct.camera,
                                moveDir[0],
                                moveDir[1],
                                moveDir[2],
                                (0.5 * direct.dr.mouseDeltaX *
                                 direct.dr.fovH),
                                0.0, 0.0)
        return Task.cont

    def HPPanTask(self, state):
        direct.camera.setHpr(direct.camera,
                             (0.5 * direct.dr.mouseDeltaX *
                              direct.dr.fovH),
                             (-0.5 * direct.dr.mouseDeltaY *
                              direct.dr.fovV),
                             0.0)
        return Task.cont

    def spawnMouseRotateTask(self):
        # Kill any existing tasks
        taskMgr.remove('manipulateCamera')
        # Set at markers position in render coordinates
        self.camManipRef.setPos(self.coaMarkerPos)
        self.camManipRef.setHpr(direct.camera, ZERO_POINT)
        t = Task.Task(self.mouseRotateTask)
        if abs(direct.dr.mouseX) > 0.9:
            t.constrainedDir = 'y'
        else:
            t.constrainedDir = 'x'
        taskMgr.add(t, 'manipulateCamera')

    def mouseRotateTask(self, state):
        # If moving outside of center, ignore motion perpendicular to edge
        if ((state.constrainedDir == 'y') and (abs(direct.dr.mouseX) > 0.9)):
            deltaX = 0
            deltaY = direct.dr.mouseDeltaY
        elif ((state.constrainedDir == 'x') and (abs(direct.dr.mouseY) > 0.9)):
            deltaX = direct.dr.mouseDeltaX
            deltaY = 0
        else:
            deltaX = direct.dr.mouseDeltaX
            deltaY = direct.dr.mouseDeltaY
        if direct.fShift:
            direct.camera.setHpr(direct.camera,
                                 (deltaX * direct.dr.fovH),
                                 (-deltaY * direct.dr.fovV),
                                 0.0)
            self.camManipRef.setPos(self.coaMarkerPos)
            self.camManipRef.setHpr(direct.camera, ZERO_POINT)
        else:
            wrt = direct.camera.getTransform( self.camManipRef )
            self.camManipRef.setHpr(self.camManipRef,
                                    (-1 * deltaX * 180.0),
                                    (deltaY * 180.0),
                                    0.0)
            direct.camera.setTransform(self.camManipRef, wrt)
        return Task.cont

    def spawnMouseRollTask(self):
        # Kill any existing tasks
        taskMgr.remove('manipulateCamera')
        # Set at markers position in render coordinates
        self.camManipRef.setPos(self.coaMarkerPos)
        self.camManipRef.setHpr(direct.camera, ZERO_POINT)
        t = Task.Task(self.mouseRollTask)
        t.coaCenter = getScreenXY(self.coaMarker)
        t.lastAngle = getCrankAngle(t.coaCenter)
        # Store the camera/manipRef offset transform
        t.wrt = direct.camera.getTransform( self.camManipRef )
        taskMgr.add(t, 'manipulateCamera')

    def mouseRollTask(self, state):
        wrt = state.wrt
        angle = getCrankAngle(state.coaCenter)
        deltaAngle = angle - state.lastAngle
        state.lastAngle = angle
        if base.config.GetBool('temp-hpr-fix',0):
            self.camManipRef.setHpr(self.camManipRef, 0, 0, deltaAngle)
        else:
            self.camManipRef.setHpr(self.camManipRef, 0, 0, -deltaAngle)
        direct.camera.setTransform(self.camManipRef, wrt)
        return Task.cont

    def lockCOA(self):
        self.fLockCOA = 1
        direct.message('COA Lock On')
            
    def unlockCOA(self):
        self.fLockCOA = 0
        direct.message('COA Lock Off')

    def toggleCOALock(self):
        self.fLockCOA = 1 - self.fLockCOA
        if self.fLockCOA:
            direct.message('COA Lock On')
        else:
            direct.message('COA Lock Off')

    def pickNextCOA(self):
        """ Cycle through collision handler entries """
        if self.cqEntries:
            # Get next entry and rotate entries
            entry = self.cqEntries[0]
            self.cqEntries = self.cqEntries[1:] + self.cqEntries[:1]
            # Filter out object's under camera
            node = entry.getIntoNode()
            nodePath = render.findPathTo(node)
            if direct.camera not in nodePath.getAncestry():
                # Compute hit point
                # KEH: use current display region ray
                # hitPt = direct.iRay.parentToHitPt(entry)
                hitPt = direct.drList.getCurrentDr().iRay.parentToHitPt(entry)
                # Move coa marker to new point
                self.updateCoa(hitPt, ref = self.coaMarkerRef)
            else:
                # Remove offending entry
                self.cqEntries = self.cqEntries[:-1]
                self.pickNextCOA()

    def computeCOA(self, node, hitPt, hitPtDist):
        coa = Point3(0)
        dr = direct.drList.getCurrentDr()
        if self.fLockCOA:
            # COA is locked, use existing point
            # Use existing point
            coa.assign(self.coaMarker.getPos(direct.camera))
            # Reset hit point count
            self.nullHitPointCount = 0
        elif node:
            # Got a hit point (hit point is in camera coordinates)
            # Set center of action
            coa.assign(hitPt)
            # Handle case of bad coa point (too close or too far)
            if ((hitPtDist < (1.1 * dr.near)) or
                (hitPtDist > dr.far)):
                # Just use existing point
                coa.assign(self.coaMarker.getPos(direct.camera))
            # Reset hit point count
            self.nullHitPointCount = 0
        else:
            # Increment null hit point count
            self.nullHitPointCount = (self.nullHitPointCount + 1) % 7
            # No COA lock and no intersection point
            # Use a point out in front of camera
            # Distance to point increases on multiple null hit points
            # MRM: Would be nice to be able to control this
            # At least display it
            dist = pow(10.0, self.nullHitPointCount)
            direct.message('COA Distance: ' + `dist`)
            coa.set(0,dist,0)
        # Compute COA Dist
        coaDist = Vec3(coa - ZERO_POINT).length()
        if coaDist < (1.1 * dr.near):
            coa.set(0,100,0)
            coaDist = 100
        # Update coa and marker
        self.updateCoa(coa, coaDist = coaDist)

    def updateCoa(self, ref2point, coaDist = None, ref = None):
        self.coa.set(ref2point[0], ref2point[1], ref2point[2])
        if not coaDist:
            coaDist = Vec3(self.coa - ZERO_POINT).length()
        # Place the marker in render space
        if ref == None:
            # KEH: use the current display region
            # ref = base.cam
            ref = direct.drList.getCurrentDr().cam
        self.coaMarker.setPos(ref, self.coa)
        pos = self.coaMarker.getPos()
        self.coaMarker.setPosHprScale(pos, Vec3(0), Vec3(1))
        # Resize it
        self.updateCoaMarkerSize(coaDist)
        # Record marker pos in render space
        self.coaMarkerPos.assign(self.coaMarker.getPos())

    def updateCoaMarkerSizeOnDeath(self, state):
        # Needed because tasks pass in state as first arg
        self.updateCoaMarkerSize()

    def updateCoaMarkerSize(self, coaDist = None):
        if not coaDist:
            coaDist = Vec3(self.coaMarker.getPos( direct.camera )).length()
        # KEH: use current display region for fov
        # sf = COA_MARKER_SF * coaDist * math.tan(deg2Rad(direct.dr.fovV))
        sf = COA_MARKER_SF * coaDist * math.tan(deg2Rad(direct.drList.getCurrentDr().fovV))
        if sf == 0.0:
            sf = 0.1
        self.coaMarker.setScale(sf)
        # Lerp color to fade out
        self.coaMarker.lerpColor(VBase4(1,0,0,1), VBase4(1,0,0,0), 3.0,
                                 task = 'fadeAway')

    def homeCam(self):
        # Record undo point
        direct.pushUndo([direct.camera])
        direct.camera.reparentTo(render)
        direct.camera.clearMat()
        # Resize coa marker
        self.updateCoaMarkerSize()

    def uprightCam(self):
        taskMgr.remove('manipulateCamera')
        # Record undo point
        direct.pushUndo([direct.camera])
        # Pitch camera till upright
        currH = direct.camera.getH()
        direct.camera.lerpHpr(currH, 0, 0,
                              CAM_MOVE_DURATION,
                              other = render,
                              blendType = 'easeInOut',
                              task = 'manipulateCamera')

    def orbitUprightCam(self):
        taskMgr.remove('manipulateCamera')
        # Record undo point
        direct.pushUndo([direct.camera])
        # Transform camera z axis to render space
        mCam2Render = Mat4()
        mCam2Render.assign(direct.camera.getMat(render))
        zAxis = Vec3(mCam2Render.xformVec(Z_AXIS))
        zAxis.normalize()
        # Compute rotation angle needed to upright cam
        orbitAngle = rad2Deg(math.acos(CLAMP(zAxis.dot(Z_AXIS),-1,1)))
        # Check angle
        if orbitAngle < 0.1:
            # Already upright
            return
        # Compute orthogonal axis of rotation
        rotAxis = Vec3(zAxis.cross(Z_AXIS))
        rotAxis.normalize()
        # Find angle between rot Axis and render X_AXIS
        rotAngle = rad2Deg(math.acos(CLAMP(rotAxis.dot(X_AXIS),-1,1)))
        # Determine sign or rotation angle
        if rotAxis[1] < 0:
            rotAngle *= -1
        # Position ref CS at coa marker with xaxis aligned with rot axis
        self.camManipRef.setPos(self.coaMarker, Vec3(0))
        self.camManipRef.setHpr(render, rotAngle, 0, 0)
        # Reparent Cam to ref Coordinate system
        parent = direct.camera.getParent()
        direct.camera.wrtReparentTo(self.camManipRef)
        # Rotate ref CS to final orientation
        t = self.camManipRef.lerpHpr(rotAngle, orbitAngle, 0,
                                     CAM_MOVE_DURATION,
                                     other = render,
                                     blendType = 'easeInOut',
                                     task = 'manipulateCamera')
        # Upon death, reparent Cam to parent
        t.parent = parent
        t.uponDeath = self.reparentCam

    def centerCam(self):
        self.centerCamIn(1.0)
        
    def centerCamNow(self):
        self.centerCamIn(0.)

    def centerCamIn(self, t):
        taskMgr.remove('manipulateCamera')
        # Record undo point
        direct.pushUndo([direct.camera])
        # Determine marker location
        markerToCam = self.coaMarker.getPos( direct.camera )
        dist = Vec3(markerToCam - ZERO_POINT).length()
        scaledCenterVec = Y_AXIS * dist
        delta = markerToCam - scaledCenterVec
        self.camManipRef.setPosHpr(direct.camera, Point3(0), Point3(0))
        t = direct.camera.lerpPos(Point3(delta),
                                  CAM_MOVE_DURATION,
                                  other = self.camManipRef,
                                  blendType = 'easeInOut',
                                  task = 'manipulateCamera')
        t.uponDeath = self.updateCoaMarkerSizeOnDeath

    def zoomCam(self, zoomFactor, t):
        taskMgr.remove('manipulateCamera')
        # Record undo point
        direct.pushUndo([direct.camera])
        # Find a point zoom factor times the current separation
        # of the widget and cam
        zoomPtToCam = self.coaMarker.getPos(direct.camera) * zoomFactor
        # Put a target nodePath there
        self.camManipRef.setPos(direct.camera, zoomPtToCam)
        # Move to that point
        t = direct.camera.lerpPos(ZERO_POINT,
                                  CAM_MOVE_DURATION,
                                  other = self.camManipRef,
                                  blendType = 'easeInOut',
                                  task = 'manipulateCamera')
        t.uponDeath = self.updateCoaMarkerSizeOnDeath
        
    def spawnMoveToView(self, view):
        # Kill any existing tasks
        taskMgr.remove('manipulateCamera')
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
        self.camManipRef.setPosHpr(self.coaMarker, ZERO_VEC,
                                   hprOffset)
        # Scale center vec by current distance to target
        offsetDistance = Vec3(direct.camera.getPos(self.camManipRef) - 
                              ZERO_POINT).length()
        scaledCenterVec = Y_AXIS * (-1.0 * offsetDistance)
        # Now put the camManipRef at that point
        self.camManipRef.setPosHpr(self.camManipRef,
                                   scaledCenterVec,
                                   ZERO_VEC)
        # Record view for next time around
        self.lastView = view
        t = direct.camera.lerpPosHpr(ZERO_POINT,
                                     VBase3(0,0,self.orthoViewRoll),
                                     CAM_MOVE_DURATION,
                                     other = self.camManipRef,
                                     blendType = 'easeInOut',
                                     task = 'manipulateCamera')
        t.uponDeath = self.updateCoaMarkerSizeOnDeath
        
        
    def swingCamAboutWidget(self, degrees, t):
        # Remove existing camera manipulation task
        taskMgr.remove('manipulateCamera')

        # Record undo point
        direct.pushUndo([direct.camera])
        
        # Coincident with widget
        self.camManipRef.setPos(self.coaMarker, ZERO_POINT)
        # But aligned with render space
        self.camManipRef.setHpr(ZERO_POINT)

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
        self.updateCoaMarkerSize()

    def fitOnWidget(self, nodePath = 'None Given'):
        # Fit the node on the screen
        # stop any ongoing tasks
        taskMgr.remove('manipulateCamera')
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

    def moveToFit(self):
        # How bit is the active widget?
        widgetScale = direct.widget.scalingNode.getScale(render)
        maxScale = max(widgetScale[0], widgetScale[1], widgetScale[2])
        # At what distance does the widget fill 50% of the screen?
        camY = ((2 * direct.dr.near * (1.5 * maxScale)) /
                min(direct.dr.nearWidth, direct.dr.nearHeight))
        # Find a point this distance along the Y axis
        # MRM: This needs to be generalized to support non uniform frusta
        centerVec = Y_AXIS * camY
        # Before moving, record the relationship between the selected nodes
        # and the widget, so that this can be maintained
        direct.selected.getWrtAll()
        # Push state onto undo stack
        direct.pushUndo(direct.selected)
        # Remove the task to keep the widget attached to the object
        taskMgr.remove('followSelectedNodePath')
        # Spawn a task to keep the selected objects with the widget
        taskMgr.add(self.stickToWidgetTask, 'stickToWidget')
        # Spawn a task to move the widget
        t = direct.widget.lerpPos(Point3(centerVec),
                                  CAM_MOVE_DURATION,
                                  other = direct.camera, 
                                  blendType = 'easeInOut',
                                  task = 'moveToFitTask')
        t.uponDeath = lambda state: taskMgr.remove('stickToWidget')

    def stickToWidgetTask(self, state):
        # Move the objects with the widget
        direct.selected.moveWrtWidgetAll()
        # Continue
        return Task.cont

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
        base.enableMouse()

    def removeManipulateCameraTask(self):
        taskMgr.remove('manipulateCamera')

