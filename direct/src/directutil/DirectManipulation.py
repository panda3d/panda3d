from PandaObject import *
from DirectGeometry import *

MANIPULATION_MOVE_DELAY = 0.65
VISIBLE_DISCS = ['x-disc-visible', 'y-disc-visible', 'z-disc-visible']

class DirectManipulationControl(PandaObject):
    def __init__(self, direct):
        # Create the grid
        self.direct = direct
        self.chan = direct.chan
        self.camera = self.chan.camera
        self.objectHandles = ObjectHandles(direct)
        self.hitPt = Point3(0)
        self.prevHit = Vec3(0)
        self.rotationCenter = Point3(0)
        self.initScaleMag = 1
        self.refNodePath = render.attachNewNode(NamedNode('refNodePath'))
        self.hitPtDist = 0
        self.constraint = None
        self.rotateAxis = 'x'
        self.lastCrankAngle = 0
        self.fSetCoa = 0
        self.fHitInit = 1
        self.fWidgetTop = 0
        self.fFreeManip = 1
        self.fScaling = 1

    def manipulationStart(self, chan):
        # Start out in select mode
        self.mode = 'select'
        # Check for a widget hit point
        numEntries = self.direct.iRay.pickWidget(
            render,chan.mouseX,chan.mouseY)
        # Did we hit a widget?
        if(numEntries):
            # Yes!
            # Entry 0 is the closest hit point if multiple hits
            minPt = 0
            # Find hit point in camera's space
            self.hitPt = self.direct.iRay.camToHitPt(minPt)
            self.hitPtDist = Vec3(self.hitPt - ZERO_POINT).length()
            # Get the associated collision queue object
            entry = self.direct.iRay.cq.getEntry(minPt)
            # Extract the node
            node = entry.getIntoNode()
            # Constraint determined by nodes name
            self.constraint = node.getName()
        else:
            # Nope, off the widget, no constraint
            self.constraint = None
        # Check to see if we are moving the object
        # We are moving the object if we either wait long enough
        """
        taskMgr.spawnTaskNamed(
            Task.doLater(MANIPULATION_MOVE_DELAY,
                         Task.Task(self.switchToMoveMode),
                         'manip-switch-to-move'),
            'manip-move-wait')
        """
        # Or if we move far enough
        self.moveDir = None
        watchMouseTask = Task.Task(self.watchMouseTask)
        watchMouseTask.initX = self.chan.mouseX
        watchMouseTask.initY = self.chan.mouseY
        taskMgr.spawnTaskNamed(watchMouseTask, 'manip-watch-mouse')

    def switchToMoveMode(self, state):
        taskMgr.removeTasksNamed('manip-watch-mouse')
        self.mode = 'move'
        self.manipulateObject()
        return Task.done

    def watchMouseTask(self, state):
        if (((abs (state.initX - self.chan.mouseX)) > 0.01) |
            ((abs (state.initY - self.chan.mouseY)) > 0.01)):
            taskMgr.removeTasksNamed('manip-move-wait')
            taskMgr.removeTasksNamed('manip-switch-to-move')
            self.mode = 'move'
            self.manipulateObject()
            return Task.done
        else:
            return Task.cont

    def manipulationStop(self):
        taskMgr.removeTasksNamed('manipulateObject')
        taskMgr.removeTasksNamed('manip-move-wait')
        taskMgr.removeTasksNamed('manip-switch-to-move')
        taskMgr.removeTasksNamed('manip-watch-mouse')
        # depending on flag.....
        if self.mode == 'select':
            # Check for object under mouse
            numEntries = self.direct.iRay.pickGeom(
                render,self.chan.mouseX,self.chan.mouseY)
            # Pick out the closest object that isn't a widget
            index = -1
            for i in range(0,numEntries):
                entry = self.direct.iRay.cq.getEntry(i)
                node = entry.getIntoNode()
                if node.isHidden():
                    pass
                # Is it a named node?, If so, see if it has a name
                elif issubclass(node.__class__, NamedNode):
                    name = node.getName()
                    if name in VISIBLE_DISCS:
                        pass
                    else:
                        index = i
                        break
                else:
                    # Not hidden and not one of the widgets, use it
                    index = i
            # Did we hit an object?
            if(index >= 0):
                # Yes!
                # Find hit point in camera's space
                self.hitPt = self.direct.iRay.camToHitPt(index)
                self.hitPtDist = Vec3(self.hitPt - ZERO_POINT).length()
                # Find the node path from the node found above
                nodePath = render.findPathDownTo(node)
                # Select it
                self.direct.select(nodePath)
            else:
                self.direct.deselectAll()
        else:
            self.manipulateObjectCleanup()

    def manipulateObjectCleanup(self):
        if self.fScaling:
            # We had been scaling, need to reset object handles
            self.objectHandles.transferObjectHandlesScale()
            self.fScaling = 0
        if self.fSetCoa:
            self.objectHandles.manipModeColor()
        self.direct.selected.highlightAll()
        self.objectHandles.showAllHandles()
        self.objectHandles.hideGuides()
        # Restart followSelectedNodePath task
        if self.direct.selected.last:
            self.spawnFollowSelectedNodePathTask()
        messenger.send('manipulateObjectCleanup')

    def spawnFollowSelectedNodePathTask(self):
        # Where are the object handles relative to the selected object
        pos = VBase3(0)
        hpr = VBase3(0)
        decomposeMatrix(self.direct.selected.last.mCoa2Dnp,
                        VBase3(0), hpr, pos, getDefaultCoordinateSystem())
        # Create the task
        t = Task.Task(self.followSelectedNodePathTask)
        # Update state variables
        t.pos = pos
        t.hpr = hpr
        t.base = self.direct.selected.last
        # Spawn the task
        taskMgr.spawnTaskNamed(t, 'followSelectedNodePath')

    def followSelectedNodePathTask(self, state):
        self.direct.widget.setPosHpr(state.base, state.pos, state.hpr)
        return Task.cont

    def enableManipulation(self):
	# Accept mouse events
	self.accept('handleMouse1', self.manipulationStart, [self.chan])
	self.accept('handleMouse1Up', self.manipulationStop)
        self.enableHotKeys()

    def enableHotKeys(self):
        self.accept(
            '.', self.objectHandles.multiplyScalingFactorBy, [2.0])
        self.accept(
            ',', self.objectHandles.multiplyScalingFactorBy, [0.5])
        self.accept('F', self.objectHandles.growToFit)

    def disableManipulation(self):
	# Ignore middle mouse events
	self.ignore('handleMouse1')
	self.ignore('handleMouse1Up')
        self.disableHotKeys()

    def disableHotKeys(self):
        self.ignore('.')
        self.ignore(',')
        self.ignore('F')

    def removeManipulateObjectTask(self):
        taskMgr.removeTasksNamed('manipulateObject')

    def manipulateObject(self):
        # Only do this if something is selected
        if self.direct.selected:
            # Remove the task to keep the widget attached to the object
            taskMgr.removeTasksNamed('followSelectedNodePath')
            # and the task to highlight the widget
            taskMgr.removeTasksNamed('highlightWidgetTask')
            # Set manipulation flag
            self.fManip = 1
            # Update object handles visibility
            self.objectHandles.showGuides()
            self.objectHandles.hideAllHandles()
            self.objectHandles.showHandle(self.constraint)
            if self.fSetCoa:
                self.objectHandles.coaModeColor()

            # Record relationship between selected nodes and widget
            self.direct.selected.getWrtAll()

            # hide the bbox of the selected objects during interaction
            self.direct.selected.dehighlightAll()

            """
            # Push the undo dcs for the selected objects
            self.direct.undo.push(
                (self.direct.selected, 'dcs'))
            """
            # Manipulate the real object with the constraint
            # The constraint is passed as the name of the node 
            self.spawnManipulateObjectTask()

    def spawnManipulateObjectTask(self):
        # reset hit-pt flag
        self.fHitInit = 1
        # record initial offset between widget and camera
        t = Task.Task(self.manipulateObjectTask)
        taskMgr.spawnTaskNamed(t, 'manipulateObject')

    def manipulateObjectTask(self, state):

        if self.constraint:
            type = self.constraint[2:]
            if type == 'post':
                self.xlate1D()
            elif type == 'disc':
                self.xlate2D()
            elif type == 'ring':
                self.rotate1D()
        elif self.fFreeManip:
            if self.fScaling & (not self.direct.fAlt):
                # We had been scaling and changed modes,
                # reset object handles
                self.objectHandles.transferObjectHandlesScale()
                self.fScaling = 0
            if self.direct.fControl:
                self.rotate2D()
            elif self.direct.fAlt:
                self.fScaling = 1
                self.scale3D()
            elif self.direct.fShift:
                self.xlateCamXY()
            else:
                self.xlateCamXZ()
        else:
            # MRM: Needed, more elegant fallback
            return Task.cont

        if self.fSetCoa:
            # Update coa based on current widget position
            self.direct.selected.last.mCoa2Dnp.assign(
                self.direct.widget.getMat(self.direct.selected.last)
                )
        else:
            # Move the objects with the widget
            self.direct.selected.moveWrtWidgetAll()
            
        # Continue
        return Task.cont

    def xlate1D(self):
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
            self.direct.widget.setPos(self.direct.widget, offset)

    def xlate2D(self):
        # Constrained 2D (planar) translation
        # Compute point of intersection of ray from eyepoint through cursor
        # to one of the three orthogonal planes on the widget.
        # This point tracks all subsequent mouse movements
        self.hitPt.assign(self.objectHandles.getWidgetIntersectPt(
            self.direct.widget, self.constraint[:1]))
        # use it to see how far to move the widget
        if self.fHitInit:
            # First time through just record hit point
            self.fHitInit = 0
            self.prevHit.assign(self.hitPt)
        else:
	    offset = self.hitPt - self.prevHit
            self.direct.widget.setPos(self.direct.widget, offset)


    def xlateCamXZ(self):
        """Constrained 2D motion parallel to the camera's image plane
        This moves the object in the camera's XZ plane"""
        # reset fHitInit
        # (in case we later switch to another manipulation mode)
        #self.fHitInit = 1
        # Where is the widget relative to current camera view
        vWidget2Camera = self.direct.widget.getPos(self.camera)
        x = vWidget2Camera[0]
        y = vWidget2Camera[1]
        z = vWidget2Camera[2]
        # Move widget (and objects) based upon mouse motion
        # Scaled up accordingly based upon widget distance
        chan = self.chan
        self.direct.widget.setX(
            self.camera,
            x + 0.5 * chan.mouseDeltaX * chan.nearWidth * (y/chan.near))
        self.direct.widget.setZ(
            self.camera,
            z + 0.5 * chan.mouseDeltaY * chan.nearHeight * (y/chan.near))

    def xlateCamXY(self):
        """Constrained 2D motion perpendicular to camera's image plane
        This moves the object in the camera's XY plane"""
        # Now, where is the widget relative to current camera view
        vWidget2Camera = self.direct.widget.getPos(self.camera)
        # If this is first time around, record initial y distance
        if self.fHitInit:
            self.fHitInit = 0
            # Record widget offset along y
            self.initY = vWidget2Camera[1]
        # Extract current values
        x = vWidget2Camera[0]
        y = vWidget2Camera[1]
        z = vWidget2Camera[2]
        # Move widget (and objects) based upon mouse motion
        # Scaled up accordingly based upon widget distance
        chan = self.chan
        self.direct.widget.setPos(
            self.camera,
            x + 0.5 * chan.mouseDeltaX * chan.nearWidth * (y/chan.near),
            y + self.initY * chan.mouseDeltaY,
            z)
    
    def getCrankAngle(self):
        # Used to compute current angle of mouse (relative to the widget's
        # origin) in screen space
        x = self.chan.mouseX - self.rotationCenter[0]
        y = self.chan.mouseY - self.rotationCenter[2]
        return (180 + rad2Deg(math.atan2(y,x)))

    def widgetCheck(self,type):
        # Utility to see if we are looking at the top or bottom of
        # a 2D planar widget or if we are looking at a 2D planar widget
        # edge on
        # Based upon angle between view vector from eye through the
        # widget's origin and one of the three principle axes
        axis = self.constraint[:1]
        # First compute vector from eye through widget origin
        mWidget2Cam = self.direct.widget.getMat(self.camera)
        # And determine where the viewpoint is relative to widget
        pos = VBase3(0)
        decomposeMatrix(mWidget2Cam, VBase3(0), VBase3(0), pos,
                        getDefaultCoordinateSystem())
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

    def getWidgetsNearProjectionPoint(self):
        # Find the position of the projection of the specified node path
        # on the near plane
        widgetOrigin = self.direct.widget.getPos(self.camera)
        # project this onto near plane
        return widgetOrigin * (self.chan.near / widgetOrigin[1])

    def getScreenXY(self):
        # Where does the widget's projection fall on the near plane
        nearVec = self.getWidgetsNearProjectionPoint()
        # Clamp these coordinates to visible screen
        nearX = self.clamp(nearVec[0], self.chan.left, self.chan.right)
        nearY = self.clamp(nearVec[2], self.chan.bottom, self.chan.top)
        # What percentage of the distance across the screen is this?
        percentX = (nearX - self.chan.left)/self.chan.nearWidth
        percentY = (nearY - self.chan.bottom)/self.chan.nearHeight
        # Map this percentage to the same -1 to 1 space as the mouse
        screenXY = Vec3((2 * percentX) - 1.0,nearVec[1],(2 * percentY) - 1.0)
        # Return the resulting value
        return screenXY

    def rotate1D(self):
        # Constrained 1D rotation about the widget's main axis (X,Y, or Z)
        # Rotation depends upon circular motion of the mouse about the
        # projection of the widget's origin on the image plane
        # A complete circle about the widget results in a change in
        # orientation of 360 degrees.

        # First initialize hit point/rotation angle
        if self.fHitInit:
            self.fHitInit = 0
            self.rotateAxis = self.constraint[:1]
            self.fWidgetTop = self.widgetCheck('top?')
            self.rotationCenter = self.getScreenXY()
            self.lastCrankAngle = self.getCrankAngle()
            
        # Rotate widget based on how far cursor has swung around origin
        newAngle = self.getCrankAngle()
        deltaAngle = self.lastCrankAngle - newAngle
        if self.fWidgetTop:
            deltaAngle = -1 * deltaAngle
        if self.rotateAxis == 'x':
            self.direct.widget.setP(self.direct.widget, deltaAngle)
        elif self.rotateAxis == 'y':
            self.direct.widget.setR(self.direct.widget, -deltaAngle)
        elif self.rotateAxis == 'z':
            self.direct.widget.setH(self.direct.widget, deltaAngle)
        # Record crank angle for next time around
        self.lastCrankAngle = newAngle

    def relHpr(self, base, h, p, r):
        # Compute widget2newWidget relative to base coordinate system
        mWidget2Base = self.direct.widget.getMat(base)
        mBase2NewBase = Mat4()
        mBase2NewBase.composeMatrix(
            UNIT_VEC, VBase3(h,p,r), ZERO_VEC,
            getDefaultCoordinateSystem())
        mBase2Widget = base.getMat(self.direct.widget)
        mWidget2Parent = self.direct.widget.getMat()
        # Compose the result
        resultMat = mWidget2Base * mBase2NewBase
        resultMat = resultMat * mBase2Widget
        resultMat = resultMat * mWidget2Parent
        # Extract and apply the hpr
        hpr = Vec3(0)
        decomposeMatrix(resultMat, VBase3(), hpr, VBase3(),
                        getDefaultCoordinateSystem())
        self.direct.widget.setHpr(hpr)

    def rotate2D(self):
        # Virtual trackball or arcball rotation of widget
        # Rotation method depends upon variable dd-want-arcball
        # Default is virtual trackball (handles 1D rotations better)
        self.fHitInit = 1
        tumbleRate = 360
        # Mouse motion edge to edge of channel results in one full turn
        self.relHpr(self.camera,
                    self.chan.mouseDeltaX * tumbleRate,
                    -self.chan.mouseDeltaY * tumbleRate,
                    0)

    def scale3D(self):
        # Scale the selected node based upon up down mouse motion
        # Mouse motion from edge to edge results in a factor of 4 scaling
        # From midpoint to edge doubles or halves objects scale
        if self.fHitInit:
            self.fHitInit = 0
            self.refNodePath.setPos(self.direct.widget, 0, 0, 0)
            self.refNodePath.setHpr(self.camera, 0, 0, 0)
            self.initScaleMag = Vec3(
                self.objectHandles.getWidgetIntersectPt(
                self.refNodePath, 'y')).length()
            # record initial scale
            self.initScale = self.direct.widget.getScale()
        # Begin
        # Scale factor is ratio current mag with init mag
        currScale = (
            self.initScale *
            (self.objectHandles.getWidgetIntersectPt(
            self.refNodePath, 'y').length() /
             self.initScaleMag)
            )
        self.direct.widget.setScale(currScale)
        
    def clamp(self, val, min, max):
        if val < min:
            return min
        elif val > max:
            return max
        else:
            return val


class ObjectHandles(NodePath,PandaObject):
    def __init__(self,direct):
        # Record pointer to direct object
        self.direct = direct
        # Initialize the superclass
        NodePath.__init__(self)

        # Load up object handles model and assign it to self
        self.assign(loader.loadModel('misc/objectHandles'))
        self.node().setName('objectHandles')
        self.scalingNode = self.getChild(0)
        self.scalingNode.node().setName('ohScalingNode')
        self.ohScalingFactor = 1.0
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
        self.xDisc.setColor(1,0,0,.2)
        self.yPostCollision.hide()
        self.yRingCollision.hide()
        self.yDisc.setColor(0,1,0,.2)
        self.zPostCollision.hide()
        self.zRingCollision.hide()
        self.zDisc.setColor(0,0,1,.2)
        # Augment geometry with lines
        self.createObjectHandleLines()
        # Create long markers to help line up in world
        self.createGuideLines()
        self.hideGuides()

    def coaModeColor(self):
        self.setColor(.5,.5,.5,1)

    def manipModeColor(self):
        self.clearColor()

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

    def setScalingFactor(self, scaleFactor):
        self.ohScalingFactor = scaleFactor
        self.scalingNode.setScale(self.ohScalingFactor)

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
        taskMgr.removeTasksNamed('resizeObjectHandles')
        sf = self.ohScalingFactor = self.ohScalingFactor * factor
        self.scalingNode.lerpScale(sf,sf,sf, 0.5,
                                   blendType = 'easeInOut',
                                   task = 'resizeObjectHandles')

    def growToFit(self):
        taskMgr.removeTasksNamed('resizeObjectHandles')
        # Increase handles scale until they cover 30% of the min dimension
        pos = self.direct.widget.getPos(self.direct.camera)
        minDim = min(self.direct.chan.nearWidth, self.direct.chan.nearHeight)
        sf = 0.15 * minDim * (pos[1]/self.direct.chan.near)
        self.ohScalingFactor = sf
        self.scalingNode.lerpScale(sf,sf,sf, 0.5,
                                   blendType = 'easeInOut',
                                   task = 'resizeObjectHandles')

    def createObjectHandleLines(self):
        # X post
        self.xPost = self.xPostGroup.attachNewNode(NamedNode('x-post-visible'))
	lines = LineNodePath(self.xPost)
	lines.setColor(VBase4(1,0,0,1))
	lines.setThickness(5)
	lines.moveTo(0,0,0)
        lines.drawTo(1.5,0,0)
        lines.create()
	lines = LineNodePath(self.xPost)
	lines.setColor(VBase4(1,0,0,1))
	lines.setThickness(1.5)
	lines.moveTo(0,0,0)
        lines.drawTo(-1.5,0,0)
        lines.create()
        
	# X ring
        self.xRing = self.xRingGroup.attachNewNode(NamedNode('x-ring-visible'))
	lines = LineNodePath(self.xRing)
	lines.setColor(VBase4(1,0,0,1))
	lines.setThickness(3)
	lines.moveTo(0,1,0)
        for ang in range(15, 370, 15):
            lines.drawTo(0,
                          math.cos(deg2Rad(ang)),
                          math.sin(deg2Rad(ang)))
        lines.create()
        
        # Y post
        self.yPost = self.yPostGroup.attachNewNode(NamedNode('y-post-visible'))
	lines = LineNodePath(self.yPost)
	lines.setColor(VBase4(0,1,0,1))
	lines.setThickness(5)
	lines.moveTo(0,0,0)
        lines.drawTo(0,1.5,0)
        lines.create()
	lines = LineNodePath(self.yPost)
	lines.setColor(VBase4(0,1,0,1))
	lines.setThickness(1.5)
	lines.moveTo(0,0,0)
        lines.drawTo(0,-1.5,0)
        lines.create()
        
	# Y ring
        self.yRing = self.yRingGroup.attachNewNode(NamedNode('y-ring-visible'))
	lines = LineNodePath(self.yRing)
	lines.setColor(VBase4(0,1,0,1))
	lines.setThickness(3)
	lines.moveTo(1,0,0)
        for ang in range(15, 370, 15):
            lines.drawTo(math.cos(deg2Rad(ang)),
                          0,
                          math.sin(deg2Rad(ang)))
        lines.create()

        # Z post
        self.zPost = self.zPostGroup.attachNewNode(NamedNode('z-post-visible'))
	lines = LineNodePath(self.zPost)
	lines.setColor(VBase4(0,0,1,1))
	lines.setThickness(5)
	lines.moveTo(0,0,0)
        lines.drawTo(0,0,1.5)
        lines.create()
	lines = LineNodePath(self.zPost)
	lines.setColor(VBase4(0,0,1,1))
	lines.setThickness(1.5)
	lines.moveTo(0,0,0)
        lines.drawTo(0,0,-1.5)
        lines.create()
        
	# Z ring
        self.zRing = self.zRingGroup.attachNewNode(NamedNode('z-ring-visible'))
	lines = LineNodePath(self.zRing)
	lines.setColor(VBase4(0,0,1,1))
	lines.setThickness(3)
	lines.moveTo(1,0,0)
        for ang in range(15, 370, 15):
            lines.drawTo(math.cos(deg2Rad(ang)),
                          math.sin(deg2Rad(ang)),
                          0)
        lines.create()

    def createGuideLines(self):
        self.guideLines = self.attachNewNode(NamedNode('guideLines'))
        # X guide lines
	lines = LineNodePath(self.guideLines)
	lines.setColor(VBase4(1,0,0,1))
	lines.setThickness(0.5)
	lines.moveTo(-500,0,0)
        lines.drawTo(500,0,0)
        lines.create()
        lines.node().setName('x-guide')

        # Y guide lines
	lines = LineNodePath(self.guideLines)
	lines.setColor(VBase4(0,1,0,1))
	lines.setThickness(0.5)
	lines.moveTo(0,-500,0)
        lines.drawTo(0,500,0)
        lines.create()
        lines.node().setName('y-guide')

        # Z guide lines
	lines = LineNodePath(self.guideLines)
	lines.setColor(VBase4(0,0,1,1))
	lines.setThickness(0.5)
	lines.moveTo(0,0,-500)
        lines.drawTo(0,0,500)
        lines.create()
        lines.node().setName('z-guide')

    def getAxisIntersectPt(self, axis):
        # Calc the xfrom from camera to widget
        mCam2Widget = self.direct.camera.getMat(self.direct.widget)
        lineDir = Vec3(mCam2Widget.xformVec(self.direct.chan.nearVec))
        lineDir.normalize()
        # And determine where the viewpoint is relative to widget
        lineOrigin = VBase3(0)
        decomposeMatrix(mCam2Widget, VBase3(0), VBase3(0), lineOrigin,
                        getDefaultCoordinateSystem())
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
        mCam2NodePath = self.direct.camera.getMat(nodePath)
    
        # And determine where the viewpoint is relative to widget
        lineOrigin = VBase3(0)
        decomposeMatrix(mCam2NodePath, VBase3(0), VBase3(0), lineOrigin,
                        getDefaultCoordinateSystem())
        
        # Next we find the vector from viewpoint to the widget through
        # the mouse's position on near plane.
        # This defines the intersection ray
        lineDir = Vec3(mCam2NodePath.xformVec(self.direct.chan.nearVec))
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



