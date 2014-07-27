from direct.showbase.DirectObject import DirectObject
from DirectGlobals import *
from DirectUtil import *
from DirectGeometry import *

COA_ORIGIN = 0
COA_CENTER = 1

# MRM: To do: handle broken node paths in selected and deselected dicts
class DirectNodePath(NodePath):
    # A node path augmented with info, bounding box, and utility methods
    def __init__(self, nodePath, bboxColor=None):
        # Initialize the superclass
        NodePath.__init__(self)
        self.assign(nodePath)
        # Create a bounding box
        self.bbox = DirectBoundingBox(self, bboxColor)
        center = self.bbox.getCenter()
        # Create matrix to hold the offset between the nodepath
        # and its center of action (COA)
        self.mCoa2Dnp = Mat4(Mat4.identMat())
        if base.direct.coaMode == COA_CENTER:
            self.mCoa2Dnp.setRow(3, Vec4(center[0], center[1], center[2], 1))

        # Transform from nodePath to widget
        self.tDnp2Widget = TransformState.makeIdentity()

    def highlight(self, fRecompute = 1):
        if fRecompute:
            pass
            #self.bbox.recompute()
        self.bbox.show()

    def dehighlight(self):
        self.bbox.hide()

    def getCenter(self):
        return self.bbox.getCenter()

    def getRadius(self):
        return self.bbox.getRadius()

    def getMin(self):
        return self.bbox.getMin()

    def getMax(self):
        return self.bbox.getMax()

class SelectedNodePaths(DirectObject):
    def __init__(self):
        self.reset()
        self.tagList = []

    def addTag(self, tag):
        if tag not in self.tagList:
            self.tagList.append(tag)

    def removeTag(self, tag):
        self.tagList.remove(tag)

    def reset(self):
        self.selectedDict = {}
        self.selectedList = [] # [gjeon] to maintain selected order
        self.deselectedDict = {}
        __builtins__["last"] = self.last = None

    def select(self, nodePath, fMultiSelect = 0, fSelectTag = 1):
        """ Select the specified node path.  Multiselect as required """
        # Do nothing if nothing selected
        if not nodePath:
            print 'Nothing selected!!'
            return None

        # Reset selected objects and highlight if multiSelect is false
        if not fMultiSelect:
            self.deselectAll()

        # Select tagged object if present
        if fSelectTag:
            for tag in self.tagList:
                if nodePath.hasNetTag(tag):
                    nodePath = nodePath.findNetTag(tag)
                    break

        # Get this pointer
        id = nodePath.get_key()
        # First see if its already in the selected dictionary
        dnp = self.getSelectedDict(id)
        # If so, deselect it
        if dnp:
            self.deselect(nodePath)
            return None
        else:
            # See if it is in the deselected dictionary
            dnp = self.getDeselectedDict(id)
            if dnp:
                # Remove it from the deselected dictionary
                del(self.deselectedDict[id])
                # Show its bounding box
                dnp.highlight()
            else:
                # Didn't find it, create a new selectedNodePath instance
                dnp = DirectNodePath(nodePath)
                # Show its bounding box
                dnp.highlight(fRecompute = 0)
            # Add it to the selected dictionary
            self.selectedDict[dnp.get_key()] = dnp
            self.selectedList.append(dnp) # [gjeon]

        # And update last
        __builtins__["last"] = self.last = dnp
        # Update cluster servers if this is a cluster client
        if base.direct.clusterMode == 'client':
            cluster.selectNodePath(dnp)
        return dnp

    def deselect(self, nodePath):
        """ Deselect the specified node path """
        # Get this pointer
        id = nodePath.get_key()
        # See if it is in the selected dictionary
        dnp = self.getSelectedDict(id)
        if dnp:
            # It was selected:
            # Hide its bounding box
            dnp.dehighlight()
            # Remove it from the selected dictionary
            del(self.selectedDict[id])
            if dnp in self.selectedList: # [gjeon]
                self.selectedList.remove(dnp)
            # And keep track of it in the deselected dictionary
            self.deselectedDict[id] = dnp
            # Send a message
            messenger.send('DIRECT_deselectedNodePath', [dnp])
            # Update cluster servers if this is a cluster client
            if base.direct.clusterMode == 'client':
                cluster.deselectNodePath(dnp)
        return dnp

    def getSelectedAsList(self):
        """
        Return a list of all selected node paths.  No verification of
        connectivity is performed on the members of the list
        """
        #return self.selectedDict.values()[:]
        return self.selectedList[:] # [gjeon] now return the list with selected order

    def __getitem__(self, index):
        return self.getSelectedAsList()[index]

    def getSelectedDict(self, id):
        """
        Search selectedDict for node path, try to repair broken node paths.
        """
        dnp = self.selectedDict.get(id, None)
        if dnp:
            return dnp
        else:
            # Not in selected dictionary
            return None

    def getDeselectedAsList(self):
        return self.deselectedDict.values()[:]

    def getDeselectedDict(self, id):
        """
        Search deselectedDict for node path, try to repair broken node paths.
        """
        dnp = self.deselectedDict.get(id, None)
        if dnp:
            # Yes
            return dnp
        else:
            # Not in deselected dictionary
            return None

    def forEachSelectedNodePathDo(self, func):
        """
        Perform given func on selected node paths.  No node path
        connectivity verification performed
        """
        selectedNodePaths = self.getSelectedAsList()
        for nodePath in selectedNodePaths:
            func(nodePath)

    def forEachDeselectedNodePathDo(self, func):
        """
        Perform given func on deselected node paths.  No node path
        connectivity verification performed
        """
        deselectedNodePaths = self.getDeselectedAsList()
        for nodePath in deselectedNodePaths:
            func(nodePath)

    def getWrtAll(self):
        self.forEachSelectedNodePathDo(self.getWrt)

    def getWrt(self, nodePath):
        nodePath.tDnp2Widget = nodePath.getTransform(base.direct.widget)

    def moveWrtWidgetAll(self):
        self.forEachSelectedNodePathDo(self.moveWrtWidget)

    def moveWrtWidget(self, nodePath):
        nodePath.setTransform(base.direct.widget, nodePath.tDnp2Widget)

    def deselectAll(self):
        self.forEachSelectedNodePathDo(self.deselect)

    def highlightAll(self):
        self.forEachSelectedNodePathDo(DirectNodePath.highlight)

    def dehighlightAll(self):
        self.forEachSelectedNodePathDo(DirectNodePath.dehighlight)

    def removeSelected(self):
        selected = self.last
        if selected:
            selected.remove()
        __builtins__["last"] = self.last = None

    def removeAll(self):
        # Remove all selected nodePaths from the Scene Graph
        self.forEachSelectedNodePathDo(NodePath.remove)

    def toggleVisSelected(self):
        selected = self.last
        # Toggle visibility of selected node paths
        if selected:
            selected.toggleVis()

    def toggleVisAll(self):
        # Toggle viz for all selected node paths
        self.forEachSelectedNodePathDo(NodePath.toggleVis)

    def isolateSelected(self):
        selected = self.last
        if selected:
            selected.isolate()

    def getDirectNodePath(self, nodePath):
        # Get this pointer
        id = nodePath.get_key()
        # First check selected dict
        dnp = self.getSelectedDict(id)
        if dnp:
            return dnp
        # Otherwise return result of deselected search
        return self.getDeselectedDict(id)

    def getNumSelected(self):
        return len(self.selectedDict.keys())


class DirectBoundingBox:
    def __init__(self, nodePath, bboxColor=None):
        # Record the node path
        self.nodePath = nodePath
        # Compute bounds, min, max, etc.
        self.computeTightBounds()
        # Generate the bounding box
        self.lines = self.createBBoxLines(bboxColor)

    def recompute(self):
        # Compute bounds, min, max, etc.
        self.computeTightBounds()
        self.updateBBoxLines()

    def computeTightBounds(self):
        # Compute bounding box using tighter calcTightBounds function
        # Need to clear out existing transform on node path
        tMat = Mat4(self.nodePath.getMat())
        self.nodePath.clearMat()
        # Get bounds
        self.min = Point3(0)
        self.max = Point3(0)
        self.nodePath.calcTightBounds(self.min, self.max)
        # Calc center and radius
        self.center = Point3((self.min + self.max)/2.0)
        self.radius = Vec3(self.max - self.min).length()
        # Restore transform
        self.nodePath.setMat(tMat)
        del tMat

    def computeBounds(self):
        self.bounds = self.getBounds()
        if self.bounds.isEmpty() or self.bounds.isInfinite():
            self.center = Point3(0)
            self.radius = 1.0
        else:
            self.center = self.bounds.getCenter()
            self.radius = self.bounds.getRadius()
        self.min = Point3(self.center - Point3(self.radius))
        self.max = Point3(self.center + Point3(self.radius))

    def createBBoxLines(self, bboxColor=None):
        # Create a line segments object for the bbox
        lines = LineNodePath(hidden)
        lines.node().setName('bboxLines')
        if (bboxColor):
            lines.setColor(VBase4(*bboxColor))
        else:
            lines.setColor(VBase4(1., 0., 0., 1.))
        lines.setThickness(0.5)

        minX = self.min[0]
        minY = self.min[1]
        minZ = self.min[2]
        maxX = self.max[0]
        maxY = self.max[1]
        maxZ = self.max[2]

        # Bottom face
        lines.moveTo(minX, minY, minZ)
        lines.drawTo(maxX, minY, minZ)
        lines.drawTo(maxX, maxY, minZ)
        lines.drawTo(minX, maxY, minZ)
        lines.drawTo(minX, minY, minZ)

        # Front Edge/Top face
        lines.drawTo(minX, minY, maxZ)
        lines.drawTo(maxX, minY, maxZ)
        lines.drawTo(maxX, maxY, maxZ)
        lines.drawTo(minX, maxY, maxZ)
        lines.drawTo(minX, minY, maxZ)

        # Three remaining edges
        lines.moveTo(maxX, minY, minZ)
        lines.drawTo(maxX, minY, maxZ)
        lines.moveTo(maxX, maxY, minZ)
        lines.drawTo(maxX, maxY, maxZ)
        lines.moveTo(minX, maxY, minZ)
        lines.drawTo(minX, maxY, maxZ)

        # Create and return bbox lines
        lines.create()

        # Make sure bbox is never lit or drawn in wireframe
        useDirectRenderStyle(lines)

        return lines

    def setBoxColorScale(self, r, g, b, a):
        if (self.lines):
            self.lines.reset()
            self.lines = None
        self.lines = self.createBBoxLines((r, g, b, a))
        self.show()

    def updateBBoxLines(self):
        ls = self.lines.lineSegs

        minX = self.min[0]
        minY = self.min[1]
        minZ = self.min[2]
        maxX = self.max[0]
        maxY = self.max[1]
        maxZ = self.max[2]

        # Bottom face
        ls.setVertex(0, minX, minY, minZ)
        ls.setVertex(1, maxX, minY, minZ)
        ls.setVertex(2, maxX, maxY, minZ)
        ls.setVertex(3, minX, maxY, minZ)
        ls.setVertex(4, minX, minY, minZ)

        # Front Edge/Top face
        ls.setVertex(5, minX, minY, maxZ)
        ls.setVertex(6, maxX, minY, maxZ)
        ls.setVertex(7, maxX, maxY, maxZ)
        ls.setVertex(8, minX, maxY, maxZ)
        ls.setVertex(9, minX, minY, maxZ)

        # Three remaining edges
        ls.setVertex(10, maxX, minY, minZ)
        ls.setVertex(11, maxX, minY, maxZ)
        ls.setVertex(12, maxX, maxY, minZ)
        ls.setVertex(13, maxX, maxY, maxZ)
        ls.setVertex(14, minX, maxY, minZ)
        ls.setVertex(15, minX, maxY, maxZ)

    def getBounds(self):
        # Get a node path's bounds
        nodeBounds = BoundingSphere()
        nodeBounds.extendBy(self.nodePath.node().getInternalBound())
        for child in self.nodePath.getChildren():
            nodeBounds.extendBy(child.getBounds())
        return nodeBounds.makeCopy()

    def show(self):
        self.lines.reparentTo(self.nodePath)

    def hide(self):
        self.lines.reparentTo(hidden)

    def getCenter(self):
        return self.center

    def getRadius(self):
        return self.radius

    def getMin(self):
        return self.min

    def getMax(self):
        return self.max

    def vecAsString(self, vec):
        return '%.2f %.2f %.2f' % (vec[0], vec[1], vec[2])

    def __repr__(self):
        return (repr(self.__class__) +
                '\nNodePath:\t%s\n' % self.nodePath.getName() +
                'Min:\t\t%s\n' % self.vecAsString(self.min) +
                'Max:\t\t%s\n' % self.vecAsString(self.max) +
                'Center:\t\t%s\n' % self.vecAsString(self.center) +
                'Radius:\t\t%.2f' % self.radius
               )


class SelectionQueue(CollisionHandlerQueue):
    def __init__(self, parentNP = None):
        if parentNP is None:
            parentNP = render
        # Initialize the superclass
        CollisionHandlerQueue.__init__(self)
        # Current index and entry in collision queue
        self.index = -1
        self.entry = None
        self.skipFlags = SKIP_NONE
        # Create a collision node path attached to the given NP
        self.collisionNodePath = NodePath(CollisionNode("collisionNP"))
        self.setParentNP(parentNP)
        # Don't pay the penalty of drawing this collision ray
        self.collisionNodePath.hide()
        self.collisionNode = self.collisionNodePath.node()
        # Intersect with geometry to begin with
        self.collideWithGeom()
        # And a traverser to do the actual collision tests
        self.ct = CollisionTraverser("DirectSelection")
        self.ct.setRespectPrevTransform(False)
        # Let the traverser know about the collision node and the queue
        self.ct.addCollider(self.collisionNodePath, self)
        # List of objects that can't be selected
        self.unpickable = UNPICKABLE
        # Derived class must add Collider to complete initialization

    def setParentNP(self, parentNP):
        # Update collisionNodePath's parent
        self.collisionNodePath.reparentTo(parentNP)

    def addCollider(self, collider):
        # Inherited class must call this function to specify collider object
        # Record collision object
        self.collider = collider
        # Add the collider to the collision Node
        self.collisionNode.addSolid(self.collider)

    def collideWithBitMask(self, bitMask):
        # The into collide mask is the bit pattern colliders look at
        # when deciding whether or not to test for a collision "into"
        # this collision solid.  Set to all Off so this collision solid
        # will not be considered in any collision tests
        self.collisionNode.setIntoCollideMask(BitMask32().allOff())
        # The from collide mask is the bit pattern *this* collision solid
        # compares against the into collide mask of candidate collision solids
        # Turn this mask all off since we're not testing for collisions against
        # collision solids
        self.collisionNode.setFromCollideMask(bitMask)

    def collideWithGeom(self):
        # The into collide mask is the bit pattern colliders look at
        # when deciding whether or not to test for a collision "into"
        # this collision solid.  Set to all Off so this collision solid
        # will not be considered in any collision tests
        self.collisionNode.setIntoCollideMask(BitMask32().allOff())
        # The from collide mask is the bit pattern *this* collision solid
        # compares against the into collide mask of candidate collision solids
        # Turn this mask all off since we're not testing for collisions against
        # collision solids
        self.collisionNode.setFromCollideMask(GeomNode.getDefaultCollideMask())

    def collideWithWidget(self):
        # This collision node should not be tested against by any other
        # collision solids
        self.collisionNode.setIntoCollideMask(BitMask32().allOff())
        # This collision node will test for collisions with any collision
        # solids with a bit mask set to 0x80000000
        mask = BitMask32()
        mask.setWord(0x80000000)
        self.collisionNode.setFromCollideMask(mask)

    def addUnpickable(self, item):
        if item not in self.unpickable:
            self.unpickable.append(item)

    def removeUnpickable(self, item):
        if item in self.unpickable:
            self.unpickable.remove(item)

    def setCurrentIndex(self, index):
        if (index < 0) or (index >= self.getNumEntries()):
            self.index = -1
        else:
            self.index = index

    def setCurrentEntry(self, entry):
        self.entry = entry

    def getCurrentEntry(self):
        return self.entry

    def isEntryBackfacing(self, entry):
        # If dot product of collision point surface normal and
        # ray from camera to collision point is positive, we are
        # looking at the backface of the polygon
        if not entry.hasSurfaceNormal():
            # Well, no way to tell.  Assume we're not backfacing.
            return 0

        if direct:
            cam = base.direct.cam
        else:
            cam = base.cam

        fromNodePath = entry.getFromNodePath()
        v = Vec3(entry.getSurfacePoint(fromNodePath))
        n = entry.getSurfaceNormal(fromNodePath)
        # Convert to camera space for backfacing test
        if self.collisionNodePath.getParent() != cam:
            # Problem: assumes base.cam is the camera in question
            p2cam = self.collisionNodePath.getParent().getMat(cam)
            v = Vec3(p2cam.xformPoint(v))
            n = p2cam.xformVec(n)
        # Normalize and check angle between to vectors
        v.normalize()
        return v.dot(n) >= 0

    def findNextCollisionEntry(self, skipFlags = SKIP_NONE):
        return self.findCollisionEntry(skipFlags, self.index + 1)

    def findCollisionEntry(self, skipFlags = SKIP_NONE, startIndex = 0):
        # Init self.index and self.entry
        self.setCurrentIndex(-1)
        self.setCurrentEntry(None)
        # Pick out the closest object that isn't a widget
        for i in range(startIndex, self.getNumEntries()):
            entry = self.getEntry(i)
            nodePath = entry.getIntoNodePath()
            if (skipFlags & SKIP_HIDDEN) and nodePath.isHidden():
                # Skip if hidden node
                pass
            elif (skipFlags & SKIP_BACKFACE) and self.isEntryBackfacing(entry):
                # Skip, if backfacing poly
                pass
            elif ((skipFlags & SKIP_CAMERA) and
                  (camera in nodePath.getAncestors())):
                # Skip if parented to a camera.
                pass
            # Can pick unpickable, use the first visible node
            elif ((skipFlags & SKIP_UNPICKABLE) and
                  (nodePath.getName() in self.unpickable)):
                # Skip if in unpickable list
                pass
            elif base.direct and\
                 ((skipFlags & SKIP_WIDGET) and
                (nodePath.getTag('WidgetName') != base.direct.widget.getName())):
                # Skip if this widget part is not belong to current widget
                pass
            elif base.direct and\
                 ((skipFlags & SKIP_WIDGET) and base.direct.fControl and
                (nodePath.getName()[2:] == 'ring')):
                # Skip when ununiformly scale in ortho view
                pass
            else:
                self.setCurrentIndex(i)
                self.setCurrentEntry(entry)
                break
        return self.getCurrentEntry()

class SelectionRay(SelectionQueue):
    def __init__(self, parentNP = None):
        if parentNP is None:
            parentNP = render
        # Initialize the superclass
        SelectionQueue.__init__(self, parentNP)
        self.addCollider(CollisionRay())

    def pick(self, targetNodePath, xy = None):
        # Determine ray direction based upon the mouse coordinates
        if xy:
            mx = xy[0]
            my = xy[1]
        elif direct:
            mx = base.direct.dr.mouseX
            my = base.direct.dr.mouseY
        else:
            if not base.mouseWatcherNode.hasMouse():
                # No mouse in window.
                self.clearEntries()
                return
            mx = base.mouseWatcherNode.getMouseX()
            my = base.mouseWatcherNode.getMouseY()

        if direct:
            self.collider.setFromLens(base.direct.camNode, mx, my)
        else:
            self.collider.setFromLens(base.camNode, mx, my)            
        self.ct.traverse(targetNodePath)
        self.sortEntries()

    def pickBitMask(self, bitMask = BitMask32.allOff(),
                    targetNodePath = None,
                    skipFlags = SKIP_ALL):
        if parentNodePath is None:
            parentNodePath = render
        self.collideWithBitMask(bitMask)
        self.pick(targetNodePath)
        # Determine collision entry
        return self.findCollisionEntry(skipFlags)

    def pickGeom(self, targetNodePath = None, skipFlags = SKIP_ALL,
                 xy = None):
        if targetNodePath is None:
            targetNodePath = render
        self.collideWithGeom()
        self.pick(targetNodePath, xy = xy)
        # Determine collision entry
        return self.findCollisionEntry(skipFlags)

    def pickWidget(self, targetNodePath = None, skipFlags = SKIP_NONE):
        if targetNodePath is None:
            targetNodePath = render
        self.collideWithWidget()
        self.pick(targetNodePath)
        # Determine collision entry
        return self.findCollisionEntry(skipFlags)

    def pick3D(self, targetNodePath, origin, dir):
        # Determine ray direction based upon the mouse coordinates
        self.collider.setOrigin(origin)
        self.collider.setDirection(dir)
        self.ct.traverse(targetNodePath)
        self.sortEntries()

    def pickGeom3D(self, targetNodePath = None,
                   origin = Point3(0), dir = Vec3(0, 0, -1),
                   skipFlags = SKIP_HIDDEN | SKIP_CAMERA):
        if targetNodePath is None:
            targetNodePath = render
        self.collideWithGeom()
        self.pick3D(targetNodePath, origin, dir)
        # Determine collision entry
        return self.findCollisionEntry(skipFlags)

    def pickBitMask3D(self, bitMask = BitMask32.allOff(),
                      targetNodePath = None,
                      origin = Point3(0), dir = Vec3(0, 0, -1),
                      skipFlags = SKIP_ALL):
        if targetNodePath is None:
            targetNodePath = render
        self.collideWithBitMask(bitMask)
        self.pick3D(targetNodePath, origin, dir)
        # Determine collision entry
        return self.findCollisionEntry(skipFlags)


class SelectionSegment(SelectionQueue):
    # Like a selection ray but with two endpoints instead of an endpoint
    # and a direction
    def __init__(self, parentNP = None, numSegments = 1):
        if parentNP is None:
            parentNP = render
        # Initialize the superclass
        SelectionQueue.__init__(self, parentNP)
        self.colliders = []
        self.numColliders = 0
        for i in range(numSegments):
            self.addCollider(CollisionSegment())

    def addCollider(self, collider):
        # Record new collision object
        self.colliders.append(collider)
        # Add the collider to the collision Node
        self.collisionNode.addSolid(collider)
        self.numColliders += 1

    def pickGeom(self, targetNodePath = None, endPointList = [],
                 skipFlags = SKIP_HIDDEN | SKIP_CAMERA):
        if targetNodePath is None:
            targetNodePath = render
        self.collideWithGeom()
        for i in range(min(len(endPointList), self.numColliders)):
            pointA, pointB = endPointList[i]
            collider = self.colliders[i]
            collider.setPointA(pointA)
            collider.setPointB(pointB)
        self.ct.traverse(targetNodePath)
        # Determine collision entry
        return self.findCollisionEntry(skipFlags)

    def pickBitMask(self, bitMask = BitMask32.allOff(),
                    targetNodePath = None, endPointList = [],
                 skipFlags = SKIP_HIDDEN | SKIP_CAMERA):
        if targetNodePath is None:
            targetNodePath = render
        self.collideWithBitMask(bitMask)
        for i in range(min(len(endPointList), self.numColliders)):
            pointA, pointB = endPointList[i]
            collider = self.colliders[i]
            collider.setPointA(pointA)
            collider.setPointB(pointB)
        self.ct.traverse(targetNodePath)
        # Determine collision entry
        return self.findCollisionEntry(skipFlags)


class SelectionSphere(SelectionQueue):
    # Wrapper around collision sphere
    def __init__(self, parentNP = None, numSpheres = 1):
        if parentNP is None:
            parentNP = render
        # Initialize the superclass
        SelectionQueue.__init__(self, parentNP)
        self.colliders = []
        self.numColliders = 0
        for i in range(numSpheres):
            self.addCollider(CollisionSphere(Point3(0), 1))

    def addCollider(self, collider):
        # Record new collision object
        self.colliders.append(collider)
        # Add the collider to the collision Node
        self.collisionNode.addSolid(collider)
        self.numColliders += 1

    def setCenter(self, i, center):
        c = self.colliders[i]
        c.setCenter(center)

    def setRadius(self, i, radius):
        c = self.colliders[i]
        c.setRadius(radius)

    def setCenterRadius(self, i, center, radius):
        c = self.colliders[i]
        c.setCenter(center)
        c.setRadius(radius)

    def isEntryBackfacing(self, entry):
        # If dot product of collision point surface normal and
        # ray from sphere origin to collision point is positive,
        # center is on the backside of the polygon
        fromNodePath = entry.getFromNodePath()
        v = Vec3(entry.getSurfacePoint(fromNodePath) -
                 entry.getFrom().getCenter())
        n = entry.getSurfaceNormal(fromNodePath)
        # If points almost on top of each other, reject face
        # (treat as backfacing)
        if v.length() < 0.05:
            return 1
        # Normalize and check angle between to vectors
        v.normalize()
        return v.dot(n) >= 0

    def pick(self, targetNodePath, skipFlags):
        self.ct.traverse(targetNodePath)
        self.sortEntries()
        return self.findCollisionEntry(skipFlags)

    def pickGeom(self, targetNodePath = None,
                 skipFlags = SKIP_HIDDEN | SKIP_CAMERA):
        if targetNodePath is None:
            targetNodePath = render
        self.collideWithGeom()
        return self.pick(targetNodePath, skipFlags)

    def pickBitMask(self, bitMask = BitMask32.allOff(),
                    targetNodePath = None,
                    skipFlags = SKIP_HIDDEN | SKIP_CAMERA):
        if targetNodePath is None:
            targetNodePath = render
        self.collideWithBitMask(bitMask)
        return self.pick(targetNodePath, skipFlags)

