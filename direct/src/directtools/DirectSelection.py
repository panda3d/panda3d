from PandaObject import *
from DirectGeometry import *
from DirectSelection import *
import __builtin__

UNPICKABLE = ['x-disc-visible', 'y-disc-visible', 'z-disc-visible',
              'gridBack', 'unpickable']

# MRM: To do: handle broken node paths in selected and deselected dicts
class DirectNodePath(NodePath):
    # A node path augmented with info, bounding box, and utility methods
    def __init__(self, nodePath):
        # Initialize the superclass
        NodePath.__init__(self)
        self.assign(nodePath)
        # Get a reasonable name
        self.name = self.getName()
        # Create a bounding box
        self.bbox = DirectBoundingBox(self)
        center = self.bbox.getCenter()
        # Create matrix to hold the offset between the nodepath
        # and its center of action (COA)
        self.mCoa2Dnp = Mat4()
        self.mCoa2Dnp.assign(Mat4.identMat())
        # self.mCoa2Dnp.setRow(3, Vec4(center[0], center[1], center[2], 1))
        # Transform from nodePath to widget
        self.mDnp2Widget = Mat4()
        self.mDnp2Widget.assign(Mat4.identMat())

    def highlight(self):
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

    def __repr__(self):
        return ('NodePath:\t%s\n' % self.name)


class SelectedNodePaths(PandaObject):
    def __init__(self):
        self.reset()

    def reset(self):
        self.selectedDict = {}
        self.deselectedDict = {}
        __builtin__.last = self.last = None

    def select(self, nodePath, fMultiSelect = 0):
        """ Select the specified node path.  Multiselect as required """
	# Do nothing if nothing selected
        if not nodePath:
            print 'Nothing selected!!'
            return None
        
	# Reset selected objects and highlight if multiSelect is false
        if not fMultiSelect:
            self.deselectAll()
        
        # Get this pointer
        id = nodePath.id()
        # First see if its already in the selected dictionary
        dnp = self.getSelectedDict(id)
        # If so, we're done
        if not dnp:
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
                dnp.highlight()
            # Add it to the selected dictionary
            self.selectedDict[dnp.id()] = dnp
        # And update last
        __builtin__.last = self.last = dnp
        return dnp

    def deselect(self, nodePath):
        """ Deselect the specified node path """
        # Get this pointer
        id = nodePath.id()
        # See if it is in the selected dictionary
        dnp = self.getSelectedDict(id)
        if dnp:
            # It was selected:
            # Hide its bounding box
            dnp.dehighlight()
            # Remove it from the selected dictionary
            del(self.selectedDict[id])
            # And keep track of it in the deselected dictionary
            self.deselectedDict[id] = dnp
            # Send a message
            messenger.send('DIRECT_deselectedNodePath', [dnp])
        return dnp

    def getSelectedAsList(self):
        """
        Return a list of all selected node paths.  No verification of
        connectivity is performed on the members of the list
        """
        return self.selectedDict.values()[:]

    def __getitem__(self,index):
        return self.getSelectedAsList()[index]

    def getSelectedDict(self, id):
        """
        Search selectedDict for node path, try to repair broken node paths.
        """
        dnp = self.selectedDict.get(id, None)
        if dnp:
            # Found item in selected Dictionary, is it still valid?
            if dnp.verifyConnectivity():
                # Yes
                return dnp
            else:
                # Not valid anymore, try to repair
                if dnp.repairConnectivity(render):
                    # Fixed, return node path
                    return dnp
                else:
                    del(self.selectedDict[id])
                    return None
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
            # Found item in deselected Dictionary, is it still valid?
            if dnp.verifyConnectivity():
                # Yes
                return dnp
            else:
                # Not valid anymore, try to repair
                if dnp.repairConnectivity(render):
                    # Fixed, return node path
                    return dnp
                else:
                    del(self.deselectedDict[id])
                    return None
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
        nodePath.mDnp2Widget.assign(nodePath.getMat(direct.widget))

    def moveWrtWidgetAll(self):
        self.forEachSelectedNodePathDo(self.moveWrtWidget)

    def moveWrtWidget(self, nodePath):
        nodePath.setMat(direct.widget, nodePath.mDnp2Widget)

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
        __builtin__.last = self.last = None
        
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
        id = nodePath.id()
        # First check selected dict
        dnp = self.getSelectedDict(id)
        if dnp:
            return dnp
        # Otherwise return result of deselected search
        return self.getDeselectedDict(id)

    def getNumSelected(self):
        return len(self.selectedDict.keys())


class DirectBoundingBox:
    def __init__(self, nodePath):
        # Record the node path
        self.nodePath = nodePath
        # Compute bounds, min, max, etc.
        self.computeBounds()
        # Generate the bounding box
        self.lines = self.createBBoxLines()

    def computeBounds(self):
        self.bounds = self.nodePath.getBounds()
        if self.bounds.isEmpty() or self.bounds.isInfinite():
            self.center = Point3(0)
            self.radius = 1.0
        else:
            self.center = self.bounds.getCenter()
            self.radius = self.bounds.getRadius()
        self.min = Point3(self.center - Point3(self.radius))
        self.max = Point3(self.center + Point3(self.radius))
        
    def createBBoxLines(self):
        # Create a line segments object for the bbox
        lines = LineNodePath(hidden)
        lines.node().setName('bboxLines')
        lines.setColor( VBase4( 1., 0., 0., 1. ) )
	lines.setThickness( 0.5 )

        minX = self.min[0]
        minY = self.min[1]
        minZ = self.min[2]
        maxX = self.max[0]
        maxY = self.max[1]
        maxZ = self.max[2]
        
        # Bottom face
	lines.moveTo( minX, minY, minZ )
	lines.drawTo( maxX, minY, minZ )
	lines.drawTo( maxX, maxY, minZ )
	lines.drawTo( minX, maxY, minZ )
	lines.drawTo( minX, minY, minZ )

	# Front Edge/Top face
	lines.drawTo( minX, minY, maxZ )
	lines.drawTo( maxX, minY, maxZ )
	lines.drawTo( maxX, maxY, maxZ )
	lines.drawTo( minX, maxY, maxZ )
	lines.drawTo( minX, minY, maxZ )

	# Three remaining edges
	lines.moveTo( maxX, minY, minZ )
	lines.drawTo( maxX, minY, maxZ )
	lines.moveTo( maxX, maxY, minZ )
	lines.drawTo( maxX, maxY, maxZ )
	lines.moveTo( minX, maxY, minZ )
	lines.drawTo( minX, maxY, maxZ )

        # Create and return bbox lines
	lines.create()
        
        # Make sure bbox is never lit or drawn in wireframe
        useDirectRenderStyle(lines)
        
        return lines

    def updateBBoxLines(self):
        ls = self.lines.lineSegs
        
        minX = self.min[0]
        minY = self.min[1]
        minZ = self.min[2]
        maxX = self.max[0]
        maxY = self.max[1]
        maxZ = self.max[2]
        
        # Bottom face
	ls.setVertex( 0, minX, minY, minZ )
	ls.setVertex( 1, maxX, minY, minZ )
	ls.setVertex( 2, maxX, maxY, minZ )
	ls.setVertex( 3, minX, maxY, minZ )
	ls.setVertex( 4, minX, minY, minZ )

	# Front Edge/Top face
	ls.setVertex( 5, minX, minY, maxZ )
	ls.setVertex( 6, maxX, minY, maxZ )
	ls.setVertex( 7, maxX, maxY, maxZ )
	ls.setVertex( 8, minX, maxY, maxZ )
	ls.setVertex( 9, minX, minY, maxZ )

	# Three remaining edges
	ls.setVertex( 10, maxX, minY, minZ )
	ls.setVertex( 11, maxX, minY, maxZ )
	ls.setVertex( 12, maxX, maxY, minZ )
	ls.setVertex( 13, maxX, maxY, maxZ )
	ls.setVertex( 14, minX, maxY, minZ )
	ls.setVertex( 15, minX, maxY, maxZ )

    def getBounds(self):
        # Get a node path's bounds
        nodeBounds = self.nodePath.node().getBound()
        for child in self.nodePath.getChildrenAsList():
            nodeBounds.extendBy(child.arc().getBound())
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
        return (`self.__class__` + 
                '\nNodePath:\t%s\n' % self.nodePath.getName() +
                'Min:\t\t%s\n' % self.vecAsString(self.min) +
                'Max:\t\t%s\n' % self.vecAsString(self.max) +
                'Center:\t\t%s\n' % self.vecAsString(self.center) +
                'Radius:\t\t%.2f' % self.radius
                )


class SelectionRay:
    def __init__(self, parent):
        # Create a collision node path attached to the given parent
        self.rayCollisionNodePath = parent.attachNewNode( CollisionNode() )
        # Don't pay the penalty of drawing this collision ray
        self.rayCollisionNodePath.hide()
        self.rayCollisionNode = self.rayCollisionNodePath.node()
        # Intersect with geometry to begin with
        self.collideWithGeom()
        # Create a collision ray
        self.ray = CollisionRay()
        # Add the ray to the collision Node
        self.rayCollisionNode.addSolid( self.ray )
        # Create a queue to hold the collision results
        self.cq = CollisionHandlerQueue()
        # Number of entries in CollisionHandlerQueue
        self.numEntries = 0
        # Current entry in collision queue
        self.cqIndex = 0
        # And a traverser to do the actual collision tests
        self.ct = CollisionTraverser( RenderRelation.getClassType() )
        # Let the traverser know about the queue and the collision node
        self.ct.addCollider(self.rayCollisionNode, self.cq )
        # List of objects that can't be selected
        self.unpickable = UNPICKABLE

    def addUnpickable(self, item):
        if item not in self.unpickable:
            self.unpickable.append(item)

    def removeUnpickable(self, item):
        if item in self.unpickable:
            self.unpickable.remove(item)

    def pickGeom(self, targetNodePath = render, fIntersectUnpickable = 0):
        self.collideWithGeom()
        self.pick(targetNodePath,
                  direct.dr.mouseX,
                  direct.dr.mouseY)
        # Init self.cqIndex
        self.cqIndex = -1
        # Pick out the closest object that isn't a widget
        for i in range(0,self.numEntries):
            entry = self.cq.getEntry(i)
            node = entry.getIntoNode()
            # Don't pick hidden nodes
            if node.isHidden():
                pass
            # Can pick unpickable, use the first visible node
            elif fIntersectUnpickable:
                self.cqIndex = i
                break
            # Is it a named node?, If so, see if it has a name
            elif issubclass(node.__class__, NamedNode):
                name = node.getName()
                if name in self.unpickable:
                    pass
                else:
                    self.cqIndex = i
                    break
            # Not hidden and not one of the widgets, use it
            else:
                self.cqIndex = i
                break
        # Did we hit an object?
        if(self.cqIndex >= 0):
            # Yes!
            # Find hit point in parent's space
            hitPt = self.parentToHitPt(self.cqIndex)
            hitPtDist = Vec3(hitPt - ZERO_POINT).length()
            return (node, hitPt, hitPtDist)
        else:
            return (None, ZERO_POINT, 0)

    def pickWidget(self, targetNodePath = render):
        self.collideWithWidget()
        self.pick(targetNodePath,
                  direct.dr.mouseX,
                  direct.dr.mouseY)
        # Did we hit a widget?
        if self.numEntries:
            # Yes!
            # Entry 0 is the closest hit point if multiple hits
            minPt = 0
            # Find hit point in parent's space
            hitPt = self.parentToHitPt(minPt)
            hitPtDist = Vec3(hitPt).length()
            # Get the associated collision queue object
            entry = self.cq.getEntry(minPt)
            # Extract the node
            node = entry.getIntoNode()
            # Return info
            return (node, hitPt, hitPtDist)
        else:
            return (None, ZERO_POINT, 0)

    def pick(self, targetNodePath, mouseX, mouseY):
        # Determine ray direction based upon the mouse coordinates
        # Note! This has to be a cam object (of type ProjectionNode)
        self.ray.setProjection( base.cam.node(), mouseX, mouseY )
        self.ct.traverse( targetNodePath.node() )
        self.numEntries = self.cq.getNumEntries()
        self.cq.sortEntries()
        return self.numEntries

    def pickNext(self):
        if self.cqIndex >= 0:
            self.cqIndex = (self.cqIndex + 1) % self.numEntries
            entry = self.cq.getEntry(self.cqIndex)
            node = entry.getIntoNode()
            # Find hit point in parent's space
            hitPt = self.parentToHitPt(self.cqIndex)
            hitPtDist = Vec3(hitPt - ZERO_POINT).length()
            return (node, hitPt, hitPtDist)
        else:
            return (None, ZERO_POINT, 0)

    def pickGeom3D(self, targetNodePath = render, origin = Point3(0),
                   dir = Vec3(0,0,-1), fIntersectUnpickable = 0):
        self.collideWithGeom()
        numEntries = self.pick3D(targetNodePath, origin, dir)
        # Init self.cqIndex
        self.cqIndex = -1
        # Pick out the closest object that isn't a widget
        for i in range(0,numEntries):
            entry = self.cq.getEntry(i)
            node = entry.getIntoNode()
            # Don't pick hidden nodes
            if node.isHidden():
                pass
            # Can pick unpickable, use the first visible node
            elif fIntersectUnpickable:
                self.cqIndex = i
                break
            # Is it a named node?, If so, see if it has a name
            elif issubclass(node.__class__, NamedNode):
                name = node.getName()
                if name in self.unpickable:
                    pass
                else:
                    self.cqIndex = i
                    break
            # Not hidden and not one of the widgets, use it
            else:
                self.cqIndex = i
                break
        # Did we hit an object?
        if(self.cqIndex >= 0):
            # Yes!
            # Find hit point in parent's space
            hitPt = self.parentToHitPt(self.cqIndex)
            hitPtDist = Vec3(hitPt - ZERO_POINT).length()
            return (node, hitPt, hitPtDist)
        else:
            return (None, ZERO_POINT, 0)

    def pick3D(self, targetNodePath, origin, dir):
        # Determine ray direction based upon the mouse coordinates
        # Note! This has to be a cam object (of type ProjectionNode)
        self.ray.setOrigin( origin )
        self.ray.setDirection( dir )
        self.ct.traverse( targetNodePath.node() )
        self.numEntries = self.cq.getNumEntries()
        self.cq.sortEntries()
        return self.numEntries

    def collideWithGeom(self):
        self.rayCollisionNode.setIntoCollideMask(BitMask32().allOff())
        self.rayCollisionNode.setFromCollideMask(BitMask32().allOff())
        self.rayCollisionNode.setCollideGeom(1)

    def collideWithWidget(self):
        self.rayCollisionNode.setIntoCollideMask(BitMask32().allOff())
        mask = BitMask32()
        mask.setWord(0x80000000)
        self.rayCollisionNode.setFromCollideMask(mask)
        self.rayCollisionNode.setCollideGeom(0)

    def objectToHitPt(self, index):
        return self.cq.getEntry(index).getIntoIntersectionPoint()

    def parentToHitPt(self, index):
        # Get the specified entry
        entry = self.cq.getEntry(index)
        hitPt = entry.getIntoIntersectionPoint()
        # Convert point from object local space to parent's space
        return entry.getInvWrtSpace().xformPoint(hitPt)

