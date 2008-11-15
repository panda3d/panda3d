from direct.showbase.DirectObject import DirectObject
from DirectGlobals import *
from DirectUtil import *
from DirectGeometry import *
from direct.task import Task
import types

class DirectManipulationControl(DirectObject):
    def __init__(self):
        # Create the grid
        self.objectHandles = ObjectHandles()
        self.hitPt = Point3(0)
        self.prevHit = Vec3(0)
        self.rotationCenter = Point3(0)
        self.initScaleMag = 1
        self.manipRef = base.direct.group.attachNewNode('manipRef')
        self.hitPtDist = 0
        self.constraint = None
        self.rotateAxis = 'x'
        self.lastCrankAngle = 0
        self.fSetCoa = 0
        self.fHitInit = 1
        self.fScaleInit = 1
        self.fWidgetTop = 0
        self.fFreeManip = 1
        self.fScaling = 0
        self.fMovable = 1
        self.mode = None
        self.actionEvents = [
            ['DIRECT-mouse1', self.manipulationStart],
            ['DIRECT-mouse1Up', self.manipulationStop],
            ['tab', self.toggleObjectHandlesMode],
            ['.', self.objectHandles.multiplyScalingFactorBy, 2.0],
            ['>', self.objectHandles.multiplyScalingFactorBy, 2.0],
            [',', self.objectHandles.multiplyScalingFactorBy, 0.5],
            ['<', self.objectHandles.multiplyScalingFactorBy, 0.5],
            ['shift-f', self.objectHandles.growToFit],
            ['i', self.plantSelectedNodePath],
            ]
        self.defaultSkipFlags = SKIP_HIDDEN | SKIP_BACKFACE
        self.optionalSkipFlags = 0
        self.unmovableTagList = []

        # [gjeon] to enable selection while other manipulation is disabled
        self.fAllowSelectionOnly = 0
        
    def manipulationStart(self, modifiers):
        # Start out in select mode
        self.mode = 'select'

        if base.direct.cameraControl.useMayaCamControls and modifiers == 4:
            self.mode = 'camera'
        
        if self.fAllowSelectionOnly:
            return

        # Check for a widget hit point
        entry = base.direct.iRay.pickWidget()
        # Did we hit a widget?
        if entry:
            # Yes!
            self.hitPt.assign(entry.getSurfacePoint(entry.getFromNodePath()))
            self.hitPtDist = Vec3(self.hitPt).length()
            # Constraint determined by nodes name
            self.constraint = entry.getIntoNodePath().getName()
        else:
            # Nope, off the widget, no constraint
            self.constraint = None
            # [gjeon] to prohibit unwanted object movement while direct window doesn't have focus
            if base.direct.cameraControl.useMayaCamControls and not base.direct.gotControl(modifiers):
                return
        
        if not base.direct.gotAlt(modifiers):
            # Check to see if we are moving the object
            # We are moving the object if we either wait long enough
            taskMgr.doMethodLater(MANIPULATION_MOVE_DELAY,
                                  self.switchToMoveMode,
                                  'manip-move-wait')
            # Or we move far enough
            self.moveDir = None
            watchMouseTask = Task.Task(self.watchMouseTask)
            watchMouseTask.initX = base.direct.dr.mouseX
            watchMouseTask.initY = base.direct.dr.mouseY
            taskMgr.add(watchMouseTask, 'manip-watch-mouse')

    def switchToMoveMode(self, state):
        taskMgr.remove('manip-watch-mouse')
        self.mode = 'move'
        self.manipulateObject()
        return Task.done

    def watchMouseTask(self, state):
        if (((abs (state.initX - base.direct.dr.mouseX)) > 0.01) or
            ((abs (state.initY - base.direct.dr.mouseY)) > 0.01)):
            taskMgr.remove('manip-move-wait')
            self.mode = 'move'
            self.manipulateObject()
            return Task.done
        else:
            return Task.cont

    def manipulationStop(self):
        taskMgr.remove('manipulateObject')
        taskMgr.remove('manip-move-wait')
        taskMgr.remove('manip-watch-mouse')
        # depending on flag.....
        if self.mode == 'select':
            # Check for object under mouse
            # Don't intersect with hidden or backfacing objects, as well as any
            # optionally specified things
            skipFlags = self.defaultSkipFlags | self.optionalSkipFlags
            # Skip camera (and its children), unless control key is pressed
            skipFlags |= SKIP_CAMERA * (1 - base.getControl())
            entry = base.direct.iRay.pickGeom(skipFlags = skipFlags)
            if entry:
                # Record hit point information
                self.hitPt.assign(entry.getSurfacePoint(entry.getFromNodePath()))
                self.hitPtDist = Vec3(self.hitPt).length()
                # Select it
                base.direct.select(entry.getIntoNodePath(), base.direct.fShift)
            else:
                base.direct.deselectAll()
        elif self.mode == 'move':
            self.manipulateObjectCleanup()
        self.mode = None

    def manipulateObjectCleanup(self):
        if self.fScaling:
            # We had been scaling, need to reset object handles
            self.objectHandles.transferObjectHandlesScale()
            self.fScaling = 0
        base.direct.selected.highlightAll()
        self.objectHandles.showAllHandles()
        if base.direct.clusterMode == 'client':
            cluster(
                'base.direct.manipulationControl.objectHandles.showAllHandles()')
        self.objectHandles.hideGuides()
        # Restart followSelectedNodePath task
        self.spawnFollowSelectedNodePathTask()
        messenger.send('DIRECT_manipulateObjectCleanup',
                       [base.direct.selected.getSelectedAsList()])

    def spawnFollowSelectedNodePathTask(self):
        # If nothing selected, just return
        if not base.direct.selected.last:
            return
        # Clear out old task to make sure
        taskMgr.remove('followSelectedNodePath')
        # Where are the object handles relative to the selected object
        pos = VBase3(0)
        hpr = VBase3(0)
        decomposeMatrix(base.direct.selected.last.mCoa2Dnp,
                        VBase3(0), hpr, pos, CSDefault)
        # Create the task
        t = Task.Task(self.followSelectedNodePathTask)
        # Update state variables
        t.pos = pos
        t.hpr = hpr
        t.base = base.direct.selected.last
        # Spawn the task
        taskMgr.add(t, 'followSelectedNodePath')

    def followSelectedNodePathTask(self, state):
        base.direct.widget.setPosHpr(state.base, state.pos, state.hpr)
        return Task.cont

    def enableManipulation(self):
        # Accept events
        for event in self.actionEvents:
            self.accept(event[0], event[1], extraArgs = event[2:])

        self.fAllowSelectionOnly = 0

    def disableManipulation(self, allowSelectionOnly=False):
        # Ignore events
        for event in self.actionEvents:
            self.ignore(event[0])

        # [gjeon] to enable selection while other manipulation is disabled
        if allowSelectionOnly:
            self.fAllowSelectionOnly = allowSelectionOnly
            self.accept('DIRECT-mouse1', self.manipulationStart)
            self.accept('DIRECT-mouse1Up', self.manipulationStop)

        self.removeManipulateObjectTask()
        taskMgr.remove('manipulateObject')
        taskMgr.remove('manip-move-wait')
        taskMgr.remove('manip-watch-mouse')
        taskMgr.remove('highlightWidgetTask')
        taskMgr.remove('resizeObjectHandles')

    def toggleObjectHandlesMode(self):
        if self.fMovable:
            self.fSetCoa = 1 - self.fSetCoa

            if self.fSetCoa:
                self.objectHandles.coaModeColor()
            else:
                self.objectHandles.manipModeColor()
        else:
            self.objectHandles.disabledModeColor()

    def removeManipulateObjectTask(self):
        taskMgr.remove('manipulateObject')

    def enableWidgetMove(self):
        self.fMovable = 1
        if self.fSetCoa:
            self.objectHandles.coaModeColor()            
        else:
            self.objectHandles.manipModeColor()

    def disableWidgetMove(self):
        self.fMovable = 0
        self.objectHandles.disabledModeColor()        

    #--------------------------------------------------------------------------
    # Function:   get edit types list for specified objects which indicate
    #             how editable the objects are
    # Parameters: object, list of object to get edit types for
    # Changes:    none
    # Returns:    list of edit types
    #--------------------------------------------------------------------------
    def getEditTypes(self, objects):
        # See if any of the selected in the don't manipulate tag list
        editTypes = 0
        for tag in self.unmovableTagList:
            for selected in objects:
                unmovableTag = selected.getTag(tag)
                if (unmovableTag):
                    # check value of unmovableTag to see if it is
                    # completely uneditable or if it allows only certain
                    # types of editing
                    editTypes |= int(unmovableTag)
        return editTypes

    def manipulateObject(self):
        # Only do this if something is selected
        selectedList = base.direct.selected.getSelectedAsList()
        # See if any of the selected are completely uneditable
        editTypes = self.getEditTypes(selectedList)
        if (editTypes & EDIT_TYPE_UNEDITABLE == EDIT_TYPE_UNEDITABLE):
            return
        self.currEditTypes = editTypes
        if selectedList:
            # Remove the task to keep the widget attached to the object
            taskMgr.remove('followSelectedNodePath')
            # and the task to highlight the widget
            taskMgr.remove('highlightWidgetTask')
            # Set manipulation flag
            self.fManip = 1
            # Record undo point
            base.direct.pushUndo(base.direct.selected)
            # Update object handles visibility
            self.objectHandles.showGuides()
            self.objectHandles.hideAllHandles()
            self.objectHandles.showHandle(self.constraint)
            if base.direct.clusterMode == 'client':
                oh = 'base.direct.manipulationControl.objectHandles'
                cluster(oh + '.showGuides()', 0)
                cluster(oh + '.hideAllHandles()', 0)
                cluster(oh + ('.showHandle("%s")'% self.constraint), 0)
            # Record relationship between selected nodes and widget
            base.direct.selected.getWrtAll()
            # hide the bbox of the selected objects during interaction
            base.direct.selected.dehighlightAll()
            # Send event to signal start of manipulation
            messenger.send('DIRECT_manipulateObjectStart')
            # Manipulate the real object with the constraint
            # The constraint is passed as the name of the node
            self.spawnManipulateObjectTask()

    def spawnManipulateObjectTask(self):
        # reset hit-pt flag
        self.fHitInit = 1
        self.fScaleInit = 1
        # record initial offset between widget and camera
        t = Task.Task(self.manipulateObjectTask)
        t.fMouseX = abs(base.direct.dr.mouseX) > 0.9
        t.fMouseY = abs(base.direct.dr.mouseY) > 0.9
        if t.fMouseX:
            t.constrainedDir = 'y'
        else:
            t.constrainedDir = 'x'
        # Compute widget's xy coords in screen space
        t.coaCenter = getScreenXY(base.direct.widget)
        # These are used to rotate about view vector
        if t.fMouseX and t.fMouseY:
            t.lastAngle = getCrankAngle(t.coaCenter)
        taskMgr.add(t, 'manipulateObject')

    def manipulateObjectTask(self, state):
        # Widget takes precedence
        if self.constraint:
            type = self.constraint[2:]
            if type == 'post' and not self.currEditTypes & EDIT_TYPE_UNMOVABLE:
                # [gjeon] to enable non-uniform scaling
                if base.direct.fControl and not self.currEditTypes & EDIT_TYPE_UNSCALABLE:
                    self.fScaling = 1
                    self.scale1D(state)
                else:
                    self.xlate1D(state)
            elif type == 'disc' and not self.currEditTypes & EDIT_TYPE_UNMOVABLE:
                self.xlate2D(state)
            elif type == 'ring' and not self.currEditTypes & EDIT_TYPE_UNROTATABLE:
                self.rotate1D(state)
        # No widget interaction, determine free manip mode
        elif self.fFreeManip:
            # If we've been scaling and changed modes, reset object handles
            if 0 and self.fScaling and (not base.direct.fAlt):
                self.objectHandles.transferObjectHandlesScale()
                self.fScaling = 0
            # Alt key switches to a scaling mode
            if base.direct.fControl and not self.currEditTypes & EDIT_TYPE_UNSCALABLE:
                self.fScaling = 1
                self.scale3D(state)
            # Otherwise, manip mode depends on where you started
            elif state.fMouseX and state.fMouseY and not self.currEditTypes & EDIT_TYPE_UNROTATABLE:
                # In the corner, spin around camera's axis
                self.rotateAboutViewVector(state)
            elif state.fMouseX or state.fMouseY and not self.currEditTypes & EDIT_TYPE_UNMOVABLE:
                # Mouse started elsewhere in the outer frame, rotate
                self.rotate2D(state)
            elif not self.currEditTypes & EDIT_TYPE_UNMOVABLE:
                # Mouse started in central region, xlate
                # Mode depends on shift key
                if base.direct.fShift or base.direct.fControl:
                    self.xlateCamXY(state)
                else:
                    self.xlateCamXZ(state)
        if self.fSetCoa:
            # Update coa based on current widget position
            base.direct.selected.last.mCoa2Dnp.assign(
                base.direct.widget.getMat(base.direct.selected.last))
        else:
            # Move the objects with the widget
            base.direct.selected.moveWrtWidgetAll()
        # Continue
        return Task.cont

    def addTag(self, tag):
        if tag not in self.unmovableTagList:
            self.unmovableTagList.append(tag)

    def removeTag(self, tag):
        self.unmovableTagList.remove(tag)


    ### WIDGET MANIPULATION METHODS ###
    def xlate1D(self, state):
        # Constrained 1D Translation along widget axis
        # Compute nearest hit point along axis and try to keep
        # that point as close to the current mouse position as possible
        # what point on the axis is the mouse pointing at?
        self.hitPt.assign(self.objectHandles.getAxisIntersectPt(
            self.constraint[:1]))
        # use it to see how far to move the widget
        if self.fHitInit:
            # First time through, just record that point
            self.fHitInit = 0
            self.prevHit.assign(self.hitPt)
        else:
            # Move widget to keep hit point as close to mouse as possible
            offset = self.hitPt - self.prevHit
            base.direct.widget.setPos(base.direct.widget, offset)

    def xlate2D(self, state):
        # Constrained 2D (planar) translation
        # Compute point of intersection of ray from eyepoint through cursor
        # to one of the three orthogonal planes on the widget.
        # This point tracks all subsequent mouse movements
        self.hitPt.assign(self.objectHandles.getWidgetIntersectPt(
            base.direct.widget, self.constraint[:1]))
        # use it to see how far to move the widget
        if self.fHitInit:
            # First time through just record hit point
            self.fHitInit = 0
            self.prevHit.assign(self.hitPt)
        else:
            offset = self.hitPt - self.prevHit
            base.direct.widget.setPos(base.direct.widget, offset)

    def rotate1D(self, state):
        # Constrained 1D rotation about the widget's main axis (X, Y, or Z)
        # Rotation depends upon circular motion of the mouse about the
        # projection of the widget's origin on the image plane
        # A complete circle about the widget results in a change in
        # orientation of 360 degrees.

        # First initialize hit point/rotation angle
        if self.fHitInit:
            self.fHitInit = 0
            self.rotateAxis = self.constraint[:1]
            self.fWidgetTop = self.widgetCheck('top?')
            self.rotationCenter = getScreenXY(base.direct.widget)
            self.lastCrankAngle = getCrankAngle(self.rotationCenter)

        # Rotate widget based on how far cursor has swung around origin
        newAngle = getCrankAngle(self.rotationCenter)
        deltaAngle = self.lastCrankAngle - newAngle
        if self.fWidgetTop:
            deltaAngle = -1 * deltaAngle
        if self.rotateAxis == 'x':
            base.direct.widget.setP(base.direct.widget, deltaAngle)
        elif self.rotateAxis == 'y':
            base.direct.widget.setR(base.direct.widget, deltaAngle)
        elif self.rotateAxis == 'z':
            base.direct.widget.setH(base.direct.widget, deltaAngle)
        # Record crank angle for next time around
        self.lastCrankAngle = newAngle

    def widgetCheck(self, type):
        # Utility to see if we are looking at the top or bottom of
        # a 2D planar widget or if we are looking at a 2D planar widget
        # edge on
        # Based upon angle between view vector from eye through the
        # widget's origin and one of the three principle axes
        axis = self.constraint[:1]
        # First compute vector from eye through widget origin
        mWidget2Cam = base.direct.widget.getMat(base.direct.camera)
        # And determine where the viewpoint is relative to widget
        pos = VBase3(0)
        decomposeMatrix(mWidget2Cam, VBase3(0), VBase3(0), pos,
                        CSDefault)
        widgetDir = Vec3(pos)
        widgetDir.normalize()
        # Convert specified widget axis to view space
        if axis == 'x':
            widgetAxis = Vec3(mWidget2Cam.xformVec(X_AXIS))
        elif axis == 'y':
            widgetAxis = Vec3(mWidget2Cam.xformVec(Y_AXIS))
        elif axis == 'z':
            widgetAxis = Vec3(mWidget2Cam.xformVec(Z_AXIS))
        widgetAxis.normalize()
        if type == 'top?':
            # Check sign of angle between two vectors
            return (widgetDir.dot(widgetAxis) < 0.)
        elif type == 'edge?':
            # Checking to see if we are viewing edge-on
            # Check angle between two vectors
            return(abs(widgetDir.dot(widgetAxis)) < .2)

    ### FREE MANIPULATION METHODS ###
    def xlateCamXZ(self, state):
        """Constrained 2D motion parallel to the camera's image plane
        This moves the object in the camera's XZ plane"""
        # reset fHitInit in case we later switch to manip mode
        self.fHitInit = 1
        # Reset scaling init flag
        self.fScaleInit = 1
        # Where is the widget relative to current camera view
        vWidget2Camera = base.direct.widget.getPos(base.direct.camera)
        x = vWidget2Camera[0]
        y = vWidget2Camera[1]
        z = vWidget2Camera[2]
        # Move widget (and objects) based upon mouse motion
        # Scaled up accordingly based upon widget distance
        dr = base.direct.dr
        base.direct.widget.setX(
            base.direct.camera,
            x + 0.5 * dr.mouseDeltaX * dr.nearWidth * (y/dr.near))
        base.direct.widget.setZ(
            base.direct.camera,
            z + 0.5 * dr.mouseDeltaY * dr.nearHeight * (y/dr.near))

    def xlateCamXY(self, state):
        """Constrained 2D motion perpendicular to camera's image plane
        This moves the object in the camera's XY plane if shift is held
        Moves object toward camera if control is held
        """
        # Reset scaling init flag
        self.fScaleInit = 1
        # Now, where is the widget relative to current camera view
        vWidget2Camera = base.direct.widget.getPos(base.direct.camera)
        # If this is first time around, record initial y distance
        if self.fHitInit:
            self.fHitInit = 0
            # Use distance to widget to scale motion along Y
            self.xlateSF = Vec3(vWidget2Camera).length()
            # Get widget's current xy coords in screen space
            coaCenter = getNearProjectionPoint(base.direct.widget)
            self.deltaNearX = coaCenter[0] - base.direct.dr.nearVec[0]
        # Which way do we move the object?
        if base.direct.fControl:
            moveDir = Vec3(vWidget2Camera)
            # If widget is behind camera invert vector
            if moveDir[1] < 0.0:
                moveDir.assign(moveDir * -1)
            moveDir.normalize()
        else:
            moveDir = Vec3(Y_AXIS)
        # Move selected objects
        dr = base.direct.dr
        # Scale move dir
        moveDir.assign(moveDir * (2.0 * dr.mouseDeltaY * self.xlateSF))
        # Add it to current widget offset
        vWidget2Camera += moveDir
        # The object, however, stays at the same relative point to mouse in X
        vWidget2Camera.setX((dr.nearVec[0] + self.deltaNearX) *
                            (vWidget2Camera[1]/dr.near))
        # Move widget
        base.direct.widget.setPos(base.direct.camera, vWidget2Camera)

    def rotate2D(self, state):
        """ Virtual trackball rotation of widget """
        # Reset init flag in case we switch to another mode
        self.fHitInit = 1
        # Reset scaling init flag
        self.fScaleInit = 1
        tumbleRate = 360
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
        # Mouse motion edge to edge of display region results in one full turn
        relHpr(base.direct.widget, base.direct.camera, deltaX * tumbleRate,
               -deltaY * tumbleRate, 0)

    def rotateAboutViewVector(self, state):
        # Reset init flag in case we switch to another mode
        self.fHitInit = 1
        # Reset scaling init flag
        self.fScaleInit = 1
        # Compute current angle
        angle = getCrankAngle(state.coaCenter)
        deltaAngle = angle - state.lastAngle
        state.lastAngle = angle
        # Mouse motion edge to edge of display region results in one full turn
        relHpr(base.direct.widget, base.direct.camera, 0, 0, -deltaAngle)

    def scale1D(self, state):
        # [gjeon] Constrained 1D scale of the selected node based upon up down mouse motion
        if self.fScaleInit:
            self.fScaleInit = 0
            self.initScaleMag = Vec3(self.objectHandles.getAxisIntersectPt(self.constraint[:1])).length()
            # record initial scale
            self.initScale = base.direct.widget.getScale()
        # Reset fHitInitFlag
        self.fHitInit = 1

        # reset the scale of the scaling widget so the calls to
        # getAxisIntersectPt calculate the correct distance
        base.direct.widget.setScale(1,1,1)

        # Scale factor is ratio current mag with init mag
        if self.constraint[:1] == 'x':
            currScale = Vec3(self.initScale.getX() * 
                             self.objectHandles.getAxisIntersectPt('x').length() / self.initScaleMag,
                             self.initScale.getY(), self.initScale.getZ())
        elif self.constraint[:1] == 'y':
            currScale = Vec3(self.initScale.getX(),
                             self.initScale.getY() * self.objectHandles.getAxisIntersectPt('y').length() / self.initScaleMag,
                             self.initScale.getZ())
        elif self.constraint[:1] == 'z':
            currScale = Vec3(self.initScale.getX(), self.initScale.getY(),
                             self.initScale.getZ() * self.objectHandles.getAxisIntersectPt('z').length() / self.initScaleMag)

        base.direct.widget.setScale(currScale)

    def scale3D(self, state):
        # Scale the selected node based upon up down mouse motion
        # Mouse motion from edge to edge results in a factor of 4 scaling
        # From midpoint to edge doubles or halves objects scale
        if self.fScaleInit:
            self.fScaleInit = 0
            self.manipRef.setPos(base.direct.widget, 0, 0, 0)
            self.manipRef.setHpr(base.direct.camera, 0, 0, 0)
            self.initScaleMag = Vec3(
                self.objectHandles.getWidgetIntersectPt(
                self.manipRef, 'y')).length()
            # record initial scale
            self.initScale = base.direct.widget.getScale()
        # Reset fHitInitFlag
        self.fHitInit = 1
        # Begin
        # Scale factor is ratio current mag with init mag
        currScale = (
            self.initScale *
            (self.objectHandles.getWidgetIntersectPt(
            self.manipRef, 'y').length() /
             self.initScaleMag)
            )
        base.direct.widget.setScale(currScale)

    ## Utility functions ##
    def plantSelectedNodePath(self):
        """ Move selected object to intersection point of cursor on scene """
        # Check for intersection
        entry = base.direct.iRay.pickGeom(
            skipFlags = SKIP_HIDDEN | SKIP_BACKFACE | SKIP_CAMERA)
        # MRM: Need to handle moving COA
        if (entry != None) and (base.direct.selected.last != None):
            # Record undo point
            base.direct.pushUndo(base.direct.selected)
            # Record wrt matrix
            base.direct.selected.getWrtAll()
            # Move selected
            base.direct.widget.setPos(
                base.direct.camera, entry.getSurfacePoint(entry.getFromNodePath()))
            # Move all the selected objects with widget
            # Move the objects with the widget
            base.direct.selected.moveWrtWidgetAll()
            # Let everyone know that something was moved
            messenger.send('DIRECT_manipulateObjectCleanup',
                           [base.direct.selected.getSelectedAsList()])

class ObjectHandles(NodePath, DirectObject):
    def __init__(self):
        # Initialize the superclass
        NodePath.__init__(self)

        # Load up object handles model and assign it to self
        self.assign(loader.loadModel('models/misc/objectHandles'))
        self.setName('objectHandles')
        self.scalingNode = self.getChild(0)
        self.scalingNode.setName('ohScalingNode')
        self.ohScalingFactor = 1.0
        self.directScalingFactor = 1.0
        # To avoid recreating a vec every frame
        self.hitPt = Vec3(0)
        # Get a handle on the components
        self.xHandles = self.find('**/X')
        self.xPostGroup = self.xHandles.find('**/x-post-group')
        self.xPostCollision = self.xHandles.find('**/x-post')
        self.xRingGroup = self.xHandles.find('**/x-ring-group')
        self.xRingCollision = self.xHandles.find('**/x-ring')
        self.xDiscGroup = self.xHandles.find('**/x-disc-group')
        self.xDisc = self.xHandles.find('**/x-disc-visible')
        self.xDiscCollision = self.xHandles.find('**/x-disc')

        self.yHandles = self.find('**/Y')
        self.yPostGroup = self.yHandles.find('**/y-post-group')
        self.yPostCollision = self.yHandles.find('**/y-post')
        self.yRingGroup = self.yHandles.find('**/y-ring-group')
        self.yRingCollision = self.yHandles.find('**/y-ring')
        self.yDiscGroup = self.yHandles.find('**/y-disc-group')
        self.yDisc = self.yHandles.find('**/y-disc-visible')
        self.yDiscCollision = self.yHandles.find('**/y-disc')

        self.zHandles = self.find('**/Z')
        self.zPostGroup = self.zHandles.find('**/z-post-group')
        self.zPostCollision = self.zHandles.find('**/z-post')
        self.zRingGroup = self.zHandles.find('**/z-ring-group')
        self.zRingCollision = self.zHandles.find('**/z-ring')
        self.zDiscGroup = self.zHandles.find('**/z-disc-group')
        self.zDisc = self.zHandles.find('**/z-disc-visible')
        self.zDiscCollision = self.zHandles.find('**/z-disc')

        # Adjust visiblity, colors, and transparency
        self.xPostCollision.hide()
        self.xRingCollision.hide()
        self.xDisc.setColor(1, 0, 0, .2)
        self.yPostCollision.hide()
        self.yRingCollision.hide()
        self.yDisc.setColor(0, 1, 0, .2)
        self.zPostCollision.hide()
        self.zRingCollision.hide()
        self.zDisc.setColor(0, 0, 1, .2)
        # Augment geometry with lines
        self.createObjectHandleLines()
        # Create long markers to help line up in world
        self.createGuideLines()
        self.hideGuides()

        # Start with widget handles hidden
        self.fActive = 1
        self.toggleWidget()

        # Make sure object handles are never lit or drawn in wireframe
        useDirectRenderStyle(self)

    def coaModeColor(self):
        self.setColor(.5, .5, .5, 0.5, 1)

    def disabledModeColor(self):
        self.setColor(0.1,0.1,0.1,0.1,1)

    def manipModeColor(self):
        self.clearColor()
            
    def toggleWidget(self):
        if self.fActive:
            self.deactivate()
        else:
            self.activate()

    def activate(self):
        self.scalingNode.reparentTo(self)
        self.fActive = 1

    def deactivate(self):
        self.scalingNode.reparentTo(hidden)
        self.fActive = 0

    def showWidgetIfActive(self):
        if self.fActive:
            self.reparentTo(base.direct.group)

    def showWidget(self):
        self.reparentTo(base.direct.group)

    def hideWidget(self):
        self.reparentTo(hidden)

    def enableHandles(self, handles):
        if type(handles) == types.ListType:
            for handle in handles:
                self.enableHandle(handle)
        elif handles == 'x':
            self.enableHandles(['x-post','x-ring','x-disc'])
        elif handles == 'y':
            self.enableHandles(['y-post','y-ring','y-disc'])
        elif handles == 'z':
            self.enableHandles(['z-post','z-ring','z-disc'])
        elif handles == 'post':
            self.enableHandles(['x-post','y-post','z-post'])
        elif handles == 'ring':
            self.enableHandles(['x-ring','y-ring','z-ring'])
        elif handles == 'disc':
            self.enableHandles(['x-disc','y-disc','z-disc'])
        elif handles == 'all':
            self.enableHandles(['x-post','x-ring','x-disc',
                                'y-post','y-ring','y-disc',
                                'z-post','z-ring','z-disc'])

    def enableHandle(self, handle):
        if handle == 'x-post':
            self.xPostGroup.reparentTo(self.xHandles)
        elif handle == 'x-ring':
            self.xRingGroup.reparentTo(self.xHandles)
        elif handle == 'x-disc':
            self.xDiscGroup.reparentTo(self.xHandles)
        if handle == 'y-post':
            self.yPostGroup.reparentTo(self.yHandles)
        elif handle == 'y-ring':
            self.yRingGroup.reparentTo(self.yHandles)
        elif handle == 'y-disc':
            self.yDiscGroup.reparentTo(self.yHandles)
        if handle == 'z-post':
            self.zPostGroup.reparentTo(self.zHandles)
        elif handle == 'z-ring':
            self.zRingGroup.reparentTo(self.zHandles)
        elif handle == 'z-disc':
            self.zDiscGroup.reparentTo(self.zHandles)

    def disableHandles(self, handles):
        if type(handles) == types.ListType:
            for handle in handles:
                self.disableHandle(handle)
        elif handles == 'x':
            self.disableHandles(['x-post','x-ring','x-disc'])
        elif handles == 'y':
            self.disableHandles(['y-post','y-ring','y-disc'])
        elif handles == 'z':
            self.disableHandles(['z-post','z-ring','z-disc'])
        elif handles == 'post':
            self.disableHandles(['x-post','y-post','z-post'])
        elif handles == 'ring':
            self.disableHandles(['x-ring','y-ring','z-ring'])
        elif handles == 'disc':
            self.disableHandles(['x-disc','y-disc','z-disc'])
        elif handles == 'all':
            self.disableHandles(['x-post','x-ring','x-disc',
                                 'y-post','y-ring','y-disc',
                                 'z-post','z-ring','z-disc'])

    def disableHandle(self, handle):
        if handle == 'x-post':
            self.xPostGroup.reparentTo(hidden)
        elif handle == 'x-ring':
            self.xRingGroup.reparentTo(hidden)
        elif handle == 'x-disc':
            self.xDiscGroup.reparentTo(hidden)
        if handle == 'y-post':
            self.yPostGroup.reparentTo(hidden)
        elif handle == 'y-ring':
            self.yRingGroup.reparentTo(hidden)
        elif handle == 'y-disc':
            self.yDiscGroup.reparentTo(hidden)
        if handle == 'z-post':
            self.zPostGroup.reparentTo(hidden)
        elif handle == 'z-ring':
            self.zRingGroup.reparentTo(hidden)
        elif handle == 'z-disc':
            self.zDiscGroup.reparentTo(hidden)

    def showAllHandles(self):
        self.xPost.show()
        self.xRing.show()
        self.xDisc.show()
        self.yPost.show()
        self.yRing.show()
        self.yDisc.show()
        self.zPost.show()
        self.zRing.show()
        self.zDisc.show()

    def hideAllHandles(self):
        self.xPost.hide()
        self.xRing.hide()
        self.xDisc.hide()
        self.yPost.hide()
        self.yRing.hide()
        self.yDisc.hide()
        self.zPost.hide()
        self.zRing.hide()
        self.zDisc.hide()

    def showHandle(self, handle):
        if handle == 'x-post':
            self.xPost.show()
        elif handle == 'x-ring':
            self.xRing.show()
        elif handle == 'x-disc':
            self.xDisc.show()
        elif handle == 'y-post':
            self.yPost.show()
        elif handle == 'y-ring':
            self.yRing.show()
        elif handle == 'y-disc':
            self.yDisc.show()
        elif handle == 'z-post':
            self.zPost.show()
        elif handle == 'z-ring':
            self.zRing.show()
        elif handle == 'z-disc':
            self.zDisc.show()

    def showGuides(self):
        self.guideLines.show()

    def hideGuides(self):
        self.guideLines.hide()

    def setDirectScalingFactor(self, factor):
        self.directScalingFactor = factor
        self.setScalingFactor(1)

    def setScalingFactor(self, scaleFactor):
        self.ohScalingFactor = self.ohScalingFactor * scaleFactor
        self.scalingNode.setScale(self.ohScalingFactor * self.directScalingFactor)

    def getScalingFactor(self):
        return self.scalingNode.getScale()

    def transferObjectHandlesScale(self):
        # see how much object handles have been scaled
        ohs = self.getScale()
        sns = self.scalingNode.getScale()
        # Transfer this to the scaling node
        self.scalingNode.setScale(
            ohs[0] * sns[0],
            ohs[1] * sns[1],
            ohs[2] * sns[2])
        self.setScale(1)

    def multiplyScalingFactorBy(self, factor):
        taskMgr.remove('resizeObjectHandles')
        self.ohScalingFactor = self.ohScalingFactor * factor
        sf = self.ohScalingFactor * self.directScalingFactor
        self.scalingNode.lerpScale(sf, sf, sf, 0.5,
                                   blendType = 'easeInOut',
                                   task = 'resizeObjectHandles')

    def growToFit(self):
        taskMgr.remove('resizeObjectHandles')
        # Increase handles scale until they cover 30% of the min dimension
        pos = base.direct.widget.getPos(base.direct.camera)
        minDim = min(base.direct.dr.nearWidth, base.direct.dr.nearHeight)
        sf = 0.15 * minDim * (pos[1]/base.direct.dr.near)
        self.ohScalingFactor = sf
        sf = sf * self.directScalingFactor
        self.scalingNode.lerpScale(sf, sf, sf, 0.5,
                                   blendType = 'easeInOut',
                                   task = 'resizeObjectHandles')

    def createObjectHandleLines(self):
        # X post
        self.xPost = self.xPostGroup.attachNewNode('x-post-visible')
        lines = LineNodePath(self.xPost)
        lines.setColor(VBase4(1, 0, 0, 1))
        lines.setThickness(5)
        lines.moveTo(0, 0, 0)
        lines.drawTo(1.5, 0, 0)
        lines.create()
        lines = LineNodePath(self.xPost)
        lines.setColor(VBase4(1, 0, 0, 1))
        lines.setThickness(1.5)
        lines.moveTo(0, 0, 0)
        lines.drawTo(-1.5, 0, 0)
        lines.create()

        # X ring
        self.xRing = self.xRingGroup.attachNewNode('x-ring-visible')
        lines = LineNodePath(self.xRing)
        lines.setColor(VBase4(1, 0, 0, 1))
        lines.setThickness(3)
        lines.moveTo(0, 1, 0)
        for ang in range(15, 370, 15):
            lines.drawTo(0,
                          math.cos(deg2Rad(ang)),
                          math.sin(deg2Rad(ang)))
        lines.create()

        # Y post
        self.yPost = self.yPostGroup.attachNewNode('y-post-visible')
        lines = LineNodePath(self.yPost)
        lines.setColor(VBase4(0, 1, 0, 1))
        lines.setThickness(5)
        lines.moveTo(0, 0, 0)
        lines.drawTo(0, 1.5, 0)
        lines.create()
        lines = LineNodePath(self.yPost)
        lines.setColor(VBase4(0, 1, 0, 1))
        lines.setThickness(1.5)
        lines.moveTo(0, 0, 0)
        lines.drawTo(0, -1.5, 0)
        lines.create()

        # Y ring
        self.yRing = self.yRingGroup.attachNewNode('y-ring-visible')
        lines = LineNodePath(self.yRing)
        lines.setColor(VBase4(0, 1, 0, 1))
        lines.setThickness(3)
        lines.moveTo(1, 0, 0)
        for ang in range(15, 370, 15):
            lines.drawTo(math.cos(deg2Rad(ang)),
                          0,
                          math.sin(deg2Rad(ang)))
        lines.create()

        # Z post
        self.zPost = self.zPostGroup.attachNewNode('z-post-visible')
        lines = LineNodePath(self.zPost)
        lines.setColor(VBase4(0, 0, 1, 1))
        lines.setThickness(5)
        lines.moveTo(0, 0, 0)
        lines.drawTo(0, 0, 1.5)
        lines.create()
        lines = LineNodePath(self.zPost)
        lines.setColor(VBase4(0, 0, 1, 1))
        lines.setThickness(1.5)
        lines.moveTo(0, 0, 0)
        lines.drawTo(0, 0, -1.5)
        lines.create()

        # Z ring
        self.zRing = self.zRingGroup.attachNewNode('z-ring-visible')
        lines = LineNodePath(self.zRing)
        lines.setColor(VBase4(0, 0, 1, 1))
        lines.setThickness(3)
        lines.moveTo(1, 0, 0)
        for ang in range(15, 370, 15):
            lines.drawTo(math.cos(deg2Rad(ang)),
                          math.sin(deg2Rad(ang)),
                          0)
        lines.create()

    def createGuideLines(self):
        self.guideLines = self.attachNewNode('guideLines')
        # X guide lines
        lines = LineNodePath(self.guideLines)
        lines.setColor(VBase4(1, 0, 0, 1))
        lines.setThickness(0.5)
        lines.moveTo(-500, 0, 0)
        lines.drawTo(500, 0, 0)
        lines.create()
        lines.setName('x-guide')

        # Y guide lines
        lines = LineNodePath(self.guideLines)
        lines.setColor(VBase4(0, 1, 0, 1))
        lines.setThickness(0.5)
        lines.moveTo(0, -500, 0)
        lines.drawTo(0, 500, 0)
        lines.create()
        lines.setName('y-guide')

        # Z guide lines
        lines = LineNodePath(self.guideLines)
        lines.setColor(VBase4(0, 0, 1, 1))
        lines.setThickness(0.5)
        lines.moveTo(0, 0, -500)
        lines.drawTo(0, 0, 500)
        lines.create()
        lines.setName('z-guide')

    def getAxisIntersectPt(self, axis):
        # Calc the xfrom from camera to widget
        mCam2Widget = base.direct.camera.getMat(base.direct.widget)
        lineDir = Vec3(mCam2Widget.xformVec(base.direct.dr.nearVec))
        lineDir.normalize()
        # And determine where the viewpoint is relative to widget
        lineOrigin = VBase3(0)
        decomposeMatrix(mCam2Widget, VBase3(0), VBase3(0), lineOrigin,
                        CSDefault)
        # Now see where this hits the plane containing the 1D motion axis.
        # Pick the intersection plane most normal to the intersection ray
        # by comparing lineDir with plane normals.  The plane with the
        # largest dotProduct is most "normal"
        if axis == 'x':
            if (abs(lineDir.dot(Y_AXIS)) > abs(lineDir.dot(Z_AXIS))):
                self.hitPt.assign(
                    planeIntersect(lineOrigin, lineDir, ORIGIN, Y_AXIS))
            else:
                self.hitPt.assign(
                    planeIntersect(lineOrigin, lineDir, ORIGIN, Z_AXIS))
            # We really only care about the nearest point on the axis
            self.hitPt.setY(0)
            self.hitPt.setZ(0)
        elif axis == 'y':
            if (abs(lineDir.dot(X_AXIS)) > abs(lineDir.dot(Z_AXIS))):
                self.hitPt.assign(
                    planeIntersect(lineOrigin, lineDir, ORIGIN, X_AXIS))
            else:
                self.hitPt.assign(
                    planeIntersect(lineOrigin, lineDir, ORIGIN, Z_AXIS))
            # We really only care about the nearest point on the axis
            self.hitPt.setX(0)
            self.hitPt.setZ(0)
        elif axis == 'z':
            if (abs(lineDir.dot(X_AXIS)) > abs(lineDir.dot(Y_AXIS))):
                self.hitPt.assign(
                    planeIntersect(lineOrigin, lineDir, ORIGIN, X_AXIS))
            else:
                self.hitPt.assign(
                    planeIntersect(lineOrigin, lineDir, ORIGIN, Y_AXIS))
            # We really only care about the nearest point on the axis
            self.hitPt.setX(0)
            self.hitPt.setY(0)
        return self.hitPt

    def getWidgetIntersectPt(self, nodePath, plane):
        # Find out the point of interection of the ray passing though the mouse
        # with the plane containing the 2D xlation or 1D rotation widgets

        # Calc the xfrom from camera to the nodePath
        mCam2NodePath = base.direct.camera.getMat(nodePath)

        # And determine where the viewpoint is relative to widget
        lineOrigin = VBase3(0)
        decomposeMatrix(mCam2NodePath, VBase3(0), VBase3(0), lineOrigin,
                        CSDefault)

        # Next we find the vector from viewpoint to the widget through
        # the mouse's position on near plane.
        # This defines the intersection ray
        lineDir = Vec3(mCam2NodePath.xformVec(base.direct.dr.nearVec))
        lineDir.normalize()
        # Find the hit point
        if plane == 'x':
            self.hitPt.assign(planeIntersect(
                lineOrigin, lineDir, ORIGIN, X_AXIS))
        elif plane == 'y':
            self.hitPt.assign(planeIntersect(
                lineOrigin, lineDir, ORIGIN, Y_AXIS))
        elif plane == 'z':
            self.hitPt.assign(planeIntersect(
                lineOrigin, lineDir, ORIGIN, Z_AXIS))
        return self.hitPt


