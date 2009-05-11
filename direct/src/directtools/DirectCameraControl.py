from direct.showbase.DirectObject import DirectObject
from DirectUtil import *
from DirectGeometry import *
from DirectGlobals import *
from direct.interval.IntervalGlobal import Sequence, Func
from direct.directnotify import DirectNotifyGlobal
from direct.task import Task

CAM_MOVE_DURATION = 1.2
COA_MARKER_SF = 0.0075
Y_AXIS = Vec3(0, 1, 0)

class DirectCameraControl(DirectObject):
    
    notify = DirectNotifyGlobal.directNotify.newCategory('DirectCameraControl')
    
    def __init__(self):
        # Create the grid
        self.startT = 0.0
        self.startF = 0
        self.orthoViewRoll = 0.0
        self.lastView = 0
        self.coa = Point3(0, 100, 0)
        self.coaMarker = loader.loadModel('models/misc/sphere')
        self.coaMarker.setName('DirectCameraCOAMarker')
        self.coaMarker.setTransparency(1)
        self.coaMarker.setColor(1, 0, 0, 0)
        self.coaMarker.setPos(0, 100, 0)
        useDirectRenderStyle(self.coaMarker)
        self.coaMarkerPos = Point3(0)
        self.coaMarkerColorIval = None
        self.fLockCOA = 0
        self.nullHitPointCount = 0
        self.cqEntries = []
        self.coaMarkerRef = base.direct.group.attachNewNode('coaMarkerRef')
        self.camManipRef = base.direct.group.attachNewNode('camManipRef')
        t = CAM_MOVE_DURATION
        self.actionEvents = [
            ['DIRECT-mouse1', self.mouseRotateStart],
            ['DIRECT-mouse1Up', self.mouseDollyStop],
            ['DIRECT-mouse2', self.mouseFlyStart],
            ['DIRECT-mouse2Up', self.mouseFlyStop],
            ['DIRECT-mouse3', self.mouseDollyStart],
            ['DIRECT-mouse3Up', self.mouseDollyStop],
            ]
        self.keyEvents = [
            ['c', self.centerCamIn, 0.5],
            ['f', self.fitOnWidget],                  # Note: This function doesn't work as intended
            ['h', self.homeCam],
            ['shift-v', self.toggleMarkerVis],          
            ['m', self.moveToFit],                      # Note: This function doesn't work as intended; the object dissappears and screen flashes
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
        # set this to true to prevent the camera from rolling
        self.lockRoll = False
        # NIK - flag to determine whether to use maya camera controls
        self.useMayaCamControls = 0
        self.altDown = 0

    def toggleMarkerVis(self):
##        if base.direct.cameraControl.coaMarker.isHidden():
##            base.direct.cameraControl.coaMarker.show()
##        else:
##            base.direct.cameraControl.coaMarker.hide()
            
        if self.coaMarker.isHidden():
            self.coaMarker.show()
        else:
            self.coaMarker.hide()

    def mouseRotateStart(self, modifiers):
        if self.useMayaCamControls and modifiers == 4:          # alt is pressed - use maya controls
            # base.direct.pushUndo([base.direct.camera])        # Wasteful use of undo
            self.spawnMouseRotateTask()

    def mouseDollyStart(self, modifiers):
        if self.useMayaCamControls and modifiers == 4: # alt is pressed - use maya controls
            # Hide the marker for this kind of motion
            self.coaMarker.hide()
            # Record time of start of mouse interaction
            self.startT= globalClock.getFrameTime()
            self.startF = globalClock.getFrameCount()
            # Start manipulation
            self.spawnHPanYZoom()
            
    def mouseDollyStop(self):
        taskMgr.remove('manipulateCamera')

    def mouseFlyStart(self, modifiers):
        # Record undo point
        # base.direct.pushUndo([base.direct.camera])            # Wasteful use of undo
        if self.useMayaCamControls and modifiers == 4:          # alt is down, use maya controls
            # Hide the marker for this kind of motion
            self.coaMarker.hide()
            # Record time of start of mouse interaction
            self.startT= globalClock.getFrameTime()
            self.startF = globalClock.getFrameCount()
            # Start manipulation
            self.spawnXZTranslate()
            self.altDown = 1
        elif not self.useMayaCamControls:
            # Where are we in the display region?
            if ((abs(base.direct.dr.mouseX) < 0.9) and (abs(base.direct.dr.mouseY) < 0.9)):
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
                if ((abs(base.direct.dr.mouseX) > 0.9) and
                    (abs(base.direct.dr.mouseY) > 0.9)):
                    # Mouse is in corners, spawn roll task
                    self.spawnMouseRollTask()
                else:
                    # Mouse is in outer frame, spawn mouseRotateTask
                    self.spawnMouseRotateTask()
        if not modifiers == 4:
            self.altDown = 0

    def mouseFlyStop(self):
        taskMgr.remove('manipulateCamera')
        stopT = globalClock.getFrameTime()
        deltaT = stopT - self.startT
        stopF = globalClock.getFrameCount()
        deltaF = stopF - self.startF
        ## No reason this shouldn't work with Maya cam on
        # if not self.useMayaCamControls and (deltaT <= 0.25) or (deltaF <= 1):

        # Do this when not trying to manipulate camera
        if not self.altDown:
            # Check for a hit point based on
            # current mouse position
            # Allow intersection with unpickable objects
            # And then spawn task to determine mouse mode
            # Don't intersect with hidden or backfacing objects
            skipFlags = SKIP_HIDDEN | SKIP_BACKFACE
            # Skip camera (and its children), unless control key is pressed
            skipFlags |= SKIP_CAMERA * (1 - base.getControl())
            self.computeCOA(base.direct.iRay.pickGeom(skipFlags = skipFlags))
            # Record reference point
            self.coaMarkerRef.iPosHprScale(base.cam)
            # Record entries
            self.cqEntries = []
            for i in range(base.direct.iRay.getNumEntries()):
                self.cqEntries.append(base.direct.iRay.getEntry(i))
        # Show the marker
        self.coaMarker.show()
        # Resize it
        self.updateCoaMarkerSize()
        
    def mouseFlyStartTopWin(self):
        print "Moving mouse 2 in new window"
        #altIsDown = base.getAlt()
        #if altIsDown:
        #    print "Alt is down"

    def mouseFlyStopTopWin(self):
        print "Stopping mouse 2 in new window"

    def spawnXZTranslateOrHPanYZoom(self):
        # Kill any existing tasks
        taskMgr.remove('manipulateCamera')
        # Spawn the new task
        t = Task.Task(self.XZTranslateOrHPanYZoomTask)
        # For HPanYZoom
        t.zoomSF = Vec3(self.coaMarker.getPos(base.direct.camera)).length()
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
        t.zoomSF = Vec3(self.coaMarker.getPos(base.direct.camera)).length()
        taskMgr.add(t, 'manipulateCamera')

    def spawnHPPan(self):
        # Kill any existing tasks
        taskMgr.remove('manipulateCamera')
        # Spawn new task
        taskMgr.add(self.HPPanTask, 'manipulateCamera')

    def XZTranslateOrHPanYZoomTask(self, state):
        if base.direct.fShift:
            return self.XZTranslateTask(state)
        else:
            return self.HPanYZoomTask(state)

    def XZTranslateOrHPPanTask(self, state):
        if base.direct.fShift:
            # Panning action
            return self.HPPanTask(state)
        else:
            # Translation action
            return self.XZTranslateTask(state)

    def XZTranslateTask(self, state):
        coaDist = Vec3(self.coaMarker.getPos(base.direct.camera)).length()
        xlateSF = (coaDist / base.direct.dr.near)
        base.direct.camera.setPos(base.direct.camera,
                             (-0.5 * base.direct.dr.mouseDeltaX *
                              base.direct.dr.nearWidth *
                              xlateSF),
                             0.0,
                             (-0.5 * base.direct.dr.mouseDeltaY *
                              base.direct.dr.nearHeight *
                              xlateSF))
        return Task.cont

    def HPanYZoomTask(self, state):
        # If the cam is orthogonal, don't rotate or zoom.
        if (hasattr(base.direct.camera.node(), "getLens") and
            base.direct.camera.node().getLens().__class__.__name__ == "OrthographicLens"):
            return

        if base.direct.fControl:
            moveDir = Vec3(self.coaMarker.getPos(base.direct.camera))
            # If marker is behind camera invert vector
            if moveDir[1] < 0.0:
                moveDir.assign(moveDir * -1)
            moveDir.normalize()
        else:
            moveDir = Vec3(Y_AXIS)

        if self.useMayaCamControls : # use maya controls
            moveDir.assign(moveDir * ((base.direct.dr.mouseDeltaX -1.0 * base.direct.dr.mouseDeltaY) 
                                    * state.zoomSF))
            hVal = 0.0
        else:
            moveDir.assign(moveDir * (-1.0 * base.direct.dr.mouseDeltaY *
                                        state.zoomSF))
            if base.direct.dr.mouseDeltaY > 0.0:
                moveDir.setY(moveDir[1] * 1.0)

            hVal = 0.5 * base.direct.dr.mouseDeltaX * base.direct.dr.fovH

        base.direct.camera.setPosHpr(base.direct.camera,
                                moveDir[0],
                                moveDir[1],
                                moveDir[2],
                                hVal,
                                0.0, 0.0)
        if (self.lockRoll == True):
            # flatten roll
            base.direct.camera.setR(0)

        return Task.cont

    def HPPanTask(self, state):
        base.direct.camera.setHpr(base.direct.camera,
                             (0.5 * base.direct.dr.mouseDeltaX *
                              base.direct.dr.fovH),
                             (-0.5 * base.direct.dr.mouseDeltaY *
                              base.direct.dr.fovV),
                             0.0)
        return Task.cont

    def spawnMouseRotateTask(self):
        # Kill any existing tasks
        taskMgr.remove('manipulateCamera')
        # Set at markers position in render coordinates
        self.camManipRef.setPos(self.coaMarkerPos)
        self.camManipRef.setHpr(base.direct.camera, ZERO_POINT)
        t = Task.Task(self.mouseRotateTask)
        if abs(base.direct.dr.mouseX) > 0.9:
            t.constrainedDir = 'y'
        else:
            t.constrainedDir = 'x'
        taskMgr.add(t, 'manipulateCamera')

    def mouseRotateTask(self, state):
        # If the cam is orthogonal, don't rotate.
        if (hasattr(base.direct.camera.node(), "getLens") and
            base.direct.camera.node().getLens().__class__.__name__ == "OrthographicLens"):
            return
        # If moving outside of center, ignore motion perpendicular to edge
        if ((state.constrainedDir == 'y') and (abs(base.direct.dr.mouseX) > 0.9)):
            deltaX = 0
            deltaY = base.direct.dr.mouseDeltaY
        elif ((state.constrainedDir == 'x') and (abs(base.direct.dr.mouseY) > 0.9)):
            deltaX = base.direct.dr.mouseDeltaX
            deltaY = 0
        else:
            deltaX = base.direct.dr.mouseDeltaX
            deltaY = base.direct.dr.mouseDeltaY
        if base.direct.fShift:
            base.direct.camera.setHpr(base.direct.camera,
                                 (deltaX * base.direct.dr.fovH),
                                 (-deltaY * base.direct.dr.fovV),
                                 0.0)
            if (self.lockRoll == True):
                # flatten roll
                base.direct.camera.setR(0)
            self.camManipRef.setPos(self.coaMarkerPos)
            self.camManipRef.setHpr(base.direct.camera, ZERO_POINT)
        else:
            wrt = base.direct.camera.getTransform(self.camManipRef)
            self.camManipRef.setHpr(self.camManipRef,
                                    (-1 * deltaX * 180.0),
                                    (deltaY * 180.0),
                                    0.0)
            if (self.lockRoll == True):
                # flatten roll
                self.camManipRef.setR(0)
            base.direct.camera.setTransform(self.camManipRef, wrt)
        return Task.cont

    def spawnMouseRollTask(self):
        # Kill any existing tasks
        taskMgr.remove('manipulateCamera')
        # Set at markers position in render coordinates
        self.camManipRef.setPos(self.coaMarkerPos)
        self.camManipRef.setHpr(base.direct.camera, ZERO_POINT)
        t = Task.Task(self.mouseRollTask)
        t.coaCenter = getScreenXY(self.coaMarker)
        t.lastAngle = getCrankAngle(t.coaCenter)
        # Store the camera/manipRef offset transform
        t.wrt = base.direct.camera.getTransform(self.camManipRef)
        taskMgr.add(t, 'manipulateCamera')

    def mouseRollTask(self, state):
        wrt = state.wrt
        angle = getCrankAngle(state.coaCenter)
        deltaAngle = angle - state.lastAngle
        state.lastAngle = angle
        self.camManipRef.setHpr(self.camManipRef, 0, 0, deltaAngle)
        if (self.lockRoll == True):
            # flatten roll
            self.camManipRef.setR(0)
        base.direct.camera.setTransform(self.camManipRef, wrt)
        return Task.cont

    def lockCOA(self):
        self.fLockCOA = 1
        base.direct.message('COA Lock On')

    def unlockCOA(self):
        self.fLockCOA = 0
        base.direct.message('COA Lock Off')

    def toggleCOALock(self):
        self.fLockCOA = 1 - self.fLockCOA
        if self.fLockCOA:
            base.direct.message('COA Lock On')
        else:
            base.direct.message('COA Lock Off')

    def pickNextCOA(self):
        """ Cycle through collision handler entries """
        if self.cqEntries:
            # Get next entry and rotate entries
            entry = self.cqEntries[0]
            self.cqEntries = self.cqEntries[1:] + self.cqEntries[:1]
            # Filter out object's under camera
            nodePath = entry.getIntoNodePath()
            if base.direct.camera not in nodePath.getAncestors():
                # Compute new hit point
                hitPt = entry.getSurfacePoint(entry.getFromNodePath())
                # Move coa marker to new point
                self.updateCoa(hitPt, ref = self.coaMarkerRef)
            else:
                # Remove offending entry
                self.cqEntries = self.cqEntries[:-1]
                self.pickNextCOA()

    def computeCOA(self, entry):
        coa = Point3(0)
        dr = base.direct.drList.getCurrentDr()
        if self.fLockCOA:
            # COA is locked, use existing point
            # Use existing point
            coa.assign(self.coaMarker.getPos(base.direct.camera))
            # Reset hit point count
            self.nullHitPointCount = 0
        elif entry:
            # Got a hit point (hit point is in camera coordinates)
            # Set center of action
            hitPt = entry.getSurfacePoint(entry.getFromNodePath())
            hitPtDist = Vec3(hitPt).length()
            coa.assign(hitPt)
            # Handle case of bad coa point (too close or too far)
            if ((hitPtDist < (1.1 * dr.near)) or
                (hitPtDist > dr.far)):
                # Just use existing point
                coa.assign(self.coaMarker.getPos(base.direct.camera))
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
            base.direct.message('COA Distance: ' + `dist`)
            coa.set(0, dist, 0)
        # Compute COA Dist
        coaDist = Vec3(coa - ZERO_POINT).length()
        if coaDist < (1.1 * dr.near):
            coa.set(0, 100, 0)
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
            ref = base.direct.drList.getCurrentDr().cam
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
            coaDist = Vec3(self.coaMarker.getPos(base.direct.camera)).length()
        # Nominal size based on default 30 degree vertical FOV
        # Need to adjust size based on distance and current FOV
        sf = COA_MARKER_SF * coaDist * (base.direct.drList.getCurrentDr().fovV/30.0)
        if sf == 0.0:
            sf = 0.1
        self.coaMarker.setScale(sf)
        # Lerp color to fade out
        if self.coaMarkerColorIval:
            self.coaMarkerColorIval.finish()
        self.coaMarkerColorIval = Sequence(
            Func(self.coaMarker.unstash),
            self.coaMarker.colorInterval(1.5, Vec4(1, 0, 0, 0),
                                         startColor = Vec4(1, 0, 0, 1),
                                         blendType = 'easeInOut'),
            Func(self.coaMarker.stash)
            )
        self.coaMarkerColorIval.start()

    def homeCam(self):
        # Record undo point
        base.direct.pushUndo([base.direct.camera])
        base.direct.camera.reparentTo(render)
        base.direct.camera.clearMat()
        # Resize coa marker
        self.updateCoaMarkerSize()

    def uprightCam(self):
        taskMgr.remove('manipulateCamera')
        # Record undo point
        base.direct.pushUndo([base.direct.camera])
        # Pitch camera till upright
        currH = base.direct.camera.getH()
        base.direct.camera.lerpHpr(currH, 0, 0,
                              CAM_MOVE_DURATION,
                              other = render,
                              blendType = 'easeInOut',
                              task = 'manipulateCamera')

    def orbitUprightCam(self):
        taskMgr.remove('manipulateCamera')
        # Record undo point
        base.direct.pushUndo([base.direct.camera])
        # Transform camera z axis to render space
        mCam2Render = Mat4(Mat4.identMat()) # [gjeon] fixed to give required argument
        mCam2Render.assign(base.direct.camera.getMat(render))
        zAxis = Vec3(mCam2Render.xformVec(Z_AXIS))
        zAxis.normalize()
        # Compute rotation angle needed to upright cam
        orbitAngle = rad2Deg(math.acos(CLAMP(zAxis.dot(Z_AXIS), -1, 1)))
        # Check angle
        if orbitAngle < 0.1:
            # Already upright
            return
        # Compute orthogonal axis of rotation
        rotAxis = Vec3(zAxis.cross(Z_AXIS))
        rotAxis.normalize()
        # Find angle between rot Axis and render X_AXIS
        rotAngle = rad2Deg(math.acos(CLAMP(rotAxis.dot(X_AXIS), -1, 1)))
        # Determine sign or rotation angle
        if rotAxis[1] < 0:
            rotAngle *= -1
        # Position ref CS at coa marker with xaxis aligned with rot axis
        self.camManipRef.setPos(self.coaMarker, Vec3(0))
        self.camManipRef.setHpr(render, rotAngle, 0, 0)
        # Reparent Cam to ref Coordinate system
        parent = base.direct.camera.getParent()
        base.direct.camera.wrtReparentTo(self.camManipRef)
        # Rotate ref CS to final orientation
        t = self.camManipRef.lerpHpr(rotAngle, orbitAngle, 0,
                                     CAM_MOVE_DURATION,
                                     other = render,
                                     blendType = 'easeInOut',
                                     task = 'manipulateCamera')
        # Upon death, reparent Cam to parent
        t.parent = parent
        t.setUponDeath(self.reparentCam)

    def centerCam(self):
        self.centerCamIn(1.0)

    def centerCamNow(self):
        self.centerCamIn(0.)

    def centerCamIn(self, t):
        taskMgr.remove('manipulateCamera')
        # Record undo point
        base.direct.pushUndo([base.direct.camera])
        # Determine marker location
        markerToCam = self.coaMarker.getPos(base.direct.camera)
        dist = Vec3(markerToCam - ZERO_POINT).length()
        scaledCenterVec = Y_AXIS * dist
        delta = markerToCam - scaledCenterVec
        self.camManipRef.setPosHpr(base.direct.camera, Point3(0), Point3(0))
        t = base.direct.camera.lerpPos(Point3(delta),
                                  CAM_MOVE_DURATION,
                                  other = self.camManipRef,
                                  blendType = 'easeInOut',
                                  task = 'manipulateCamera')
        t.setUponDeath(self.updateCoaMarkerSizeOnDeath)

    def zoomCam(self, zoomFactor, t):
        taskMgr.remove('manipulateCamera')
        # Record undo point
        base.direct.pushUndo([base.direct.camera])
        # Find a point zoom factor times the current separation
        # of the widget and cam
        zoomPtToCam = self.coaMarker.getPos(base.direct.camera) * zoomFactor
        # Put a target nodePath there
        self.camManipRef.setPos(base.direct.camera, zoomPtToCam)
        # Move to that point
        t = base.direct.camera.lerpPos(ZERO_POINT,
                                  CAM_MOVE_DURATION,
                                  other = self.camManipRef,
                                  blendType = 'easeInOut',
                                  task = 'manipulateCamera')
        t.setUponDeath(self.updateCoaMarkerSizeOnDeath)

    def spawnMoveToView(self, view):
        # Kill any existing tasks
        taskMgr.remove('manipulateCamera')
        # Record undo point
        base.direct.pushUndo([base.direct.camera])
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
        offsetDistance = Vec3(base.direct.camera.getPos(self.camManipRef) -
                              ZERO_POINT).length()
        scaledCenterVec = Y_AXIS * (-1.0 * offsetDistance)
        # Now put the camManipRef at that point
        self.camManipRef.setPosHpr(self.camManipRef,
                                   scaledCenterVec,
                                   ZERO_VEC)
        # Record view for next time around
        self.lastView = view
        t = base.direct.camera.lerpPosHpr(ZERO_POINT,
                                     VBase3(0, 0, self.orthoViewRoll),
                                     CAM_MOVE_DURATION,
                                     other = self.camManipRef,
                                     blendType = 'easeInOut',
                                     task = 'manipulateCamera')
        t.setUponDeath(self.updateCoaMarkerSizeOnDeath)


    def swingCamAboutWidget(self, degrees, t):
        # Remove existing camera manipulation task
        taskMgr.remove('manipulateCamera')

        # Record undo point
        base.direct.pushUndo([base.direct.camera])

        # Coincident with widget
        self.camManipRef.setPos(self.coaMarker, ZERO_POINT)
        # But aligned with render space
        self.camManipRef.setHpr(ZERO_POINT)

        parent = base.direct.camera.getParent()
        base.direct.camera.wrtReparentTo(self.camManipRef)

        manipTask = self.camManipRef.lerpHpr(VBase3(degrees, 0, 0),
                                             CAM_MOVE_DURATION,
                                             blendType = 'easeInOut',
                                             task = 'manipulateCamera')
        # Upon death, reparent Cam to parent
        manipTask.parent = parent
        manipTask.setUponDeath(self.reparentCam)

    def reparentCam(self, state):
        base.direct.camera.wrtReparentTo(state.parent)
        self.updateCoaMarkerSize()

    def fitOnWidget(self, nodePath = 'None Given'):
        # Fit the node on the screen
        # stop any ongoing tasks
        taskMgr.remove('manipulateCamera')
        # How big is the node?
        nodeScale = base.direct.widget.scalingNode.getScale(render)
        maxScale = max(nodeScale[0], nodeScale[1], nodeScale[2])
        maxDim = min(base.direct.dr.nearWidth, base.direct.dr.nearHeight)

        # At what distance does the object fill 30% of the screen?
        # Assuming radius of 1 on widget
        camY = base.direct.dr.near * (2.0 * maxScale)/(0.3 * maxDim)

        # What is the vector through the center of the screen?
        centerVec = Y_AXIS * camY

        # Where is the node relative to the viewpoint
        vWidget2Camera = base.direct.widget.getPos(base.direct.camera)

        # How far do you move the camera to be this distance from the node?
        deltaMove = vWidget2Camera - centerVec
        
        # Move a target there
        try:
            self.camManipRef.setPos(base.direct.camera, deltaMove)
        except Exception:
            self.notify.debug

        parent = base.direct.camera.getParent()
        base.direct.camera.wrtReparentTo(self.camManipRef)
        fitTask = base.direct.camera.lerpPos(Point3(0, 0, 0),
                                        CAM_MOVE_DURATION,
                                        blendType = 'easeInOut',
                                        task = 'manipulateCamera')
        # Upon death, reparent Cam to parent
        fitTask.parent = parent
        fitTask.setUponDeath(self.reparentCam)

    def moveToFit(self):
        # How big is the active widget?
        widgetScale = base.direct.widget.scalingNode.getScale(render)
        maxScale = max(widgetScale[0], widgetScale[1], widgetScale[2])
        # At what distance does the widget fill 50% of the screen?
        camY = ((2 * base.direct.dr.near * (1.5 * maxScale)) /
                min(base.direct.dr.nearWidth, base.direct.dr.nearHeight))
        # Find a point this distance along the Y axis
        # MRM: This needs to be generalized to support non uniform frusta
        centerVec = Y_AXIS * camY
        # Before moving, record the relationship between the selected nodes
        # and the widget, so that this can be maintained
        base.direct.selected.getWrtAll()
        # Push state onto undo stack
        base.direct.pushUndo(base.direct.selected)
        # Remove the task to keep the widget attached to the object
        taskMgr.remove('followSelectedNodePath')
        # Spawn a task to keep the selected objects with the widget
        taskMgr.add(self.stickToWidgetTask, 'stickToWidget')
        # Spawn a task to move the widget
        t = base.direct.widget.lerpPos(Point3(centerVec),
                                  CAM_MOVE_DURATION,
                                  other = base.direct.camera,
                                  blendType = 'easeInOut',
                                  task = 'moveToFitTask')
        t.setUponDeath(lambda state: taskMgr.remove('stickToWidget'))

    def stickToWidgetTask(self, state):
        # Move the objects with the widget
        base.direct.selected.moveWrtWidgetAll()
        # Continue
        return Task.cont

    def enableMouseFly(self, fKeyEvents = 1):
        # disable C++ fly interface
        base.disableMouse()
        # Enable events
        for event in self.actionEvents:
            self.accept(event[0], event[1], extraArgs = event[2:])
        if fKeyEvents:
            for event in self.keyEvents:
                self.accept(event[0], event[1], extraArgs = event[2:])
        # Show marker
        self.coaMarker.reparentTo(base.direct.group)

    def disableMouseFly(self):
        # Hide the marker
        self.coaMarker.reparentTo(hidden)
        # Ignore events
        for event in self.actionEvents:
            self.ignore(event[0])
        for event in self.keyEvents:
            self.ignore(event[0])
        # Kill tasks
        self.removeManipulateCameraTask()
        taskMgr.remove('stickToWidget')
        base.enableMouse()

    def removeManipulateCameraTask(self):
        taskMgr.remove('manipulateCamera')

