from PandaObject import *
from DirectGeometry import *
from DirectSelection import *


class SelectionRay:
    def __init__(self, camera, fGeom = 1):
        # Record the camera associated with this selection ray
        self.camera = camera
        # Create a collision node
        self.rayCollisionNodePath = camera.attachNewNode( CollisionNode() )
        # Don't pay the penalty of drawing this collision ray
        self.rayCollisionNodePath.hide()
        rayCollisionNode = self.rayCollisionNodePath.node()
        # Specify if this ray collides with geometry
        rayCollisionNode.setCollideGeom(fGeom)
        # Create a collision ray
        self.ray = CollisionRay()
        # Add the ray to the collision Node
        rayCollisionNode.addSolid( self.ray )
        # Create a queue to hold the collision results
        self.cq = CollisionHandlerQueue()
        self.numEntries = 0
        # And a traverser to do the actual collision tests
        self.ct = CollisionTraverser( RenderRelation.getClassType() )
        # Let the traverser know about the queue and the collision node
        self.ct.addCollider(rayCollisionNode, self.cq )

    def pick(self, targetNodePath, mouseX, mouseY):
        # Determine ray direction based upon the mouse coordinates
        # Note! This has to be a cam object (of type ProjectionNode)
        self.ray.setProjection( base.cam.node(), mouseX, mouseY )
        self.ct.traverse( targetNodePath.node() )
        self.numEntries = self.cq.getNumEntries()
        return self.numEntries

    def objectToHitPt(self, index):
        return self.cq.getEntry(index).getIntoIntersectionPoint()

    def camToHitPt(self, index):
        # Get the specified entry
        entry = self.cq.getEntry(index)
        hitPt = entry.getIntoIntersectionPoint()
        # Convert point from object local space to camera space
        return entry.getInvWrtSpace().xformPoint(hitPt)


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
            nodeBounds.extendBy(child.getBottomArc().getBound())
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
                '\nNodePath:\t%s\n' % self.name +
                'Min:\t\t%s\n' % self.vecAsString(self.min) +
                'Max:\t\t%s\n' % self.vecAsString(self.max) +
                'Center:\t\t%s\n' % self.vecAsString(self.center) +
                'Radius:\t\t%.2f' % self.radius
                )


class DirectNodePath(NodePath):
    # A node path augmented with info, bounding box, and utility methods
    def __init__(self, nodePath):
        # Initialize the superclass
        NodePath.__init__(self)
        self.assign(nodePath)
        # Get a reasonable name
        self.name = self.getNodePathName()
        # Create a bounding box
        self.bbox = DirectBoundingBox(self)
        # Use value of this pointer as unique ID
        self.id = self.node().this
        # Show bounding box
        self.highlight()

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
        self.selectedDict = {}
        self.deselectedDict = {}
        self.last = None

    def select(self, nodePath, fMultiSelect = 0):
	# Do nothing if nothing selected
        if not nodePath:
            print 'Nothing selected!!'
            return None
        
	# Reset selected objects and highlight if multiSelect is false
        if not fMultiSelect:
            self.deselectAll()

        # Get this pointer
        id = nodePath.node().this
        # First see if its already in the selected dictionary
        dnp = self.selectedDict.get(id, None)
        # If so, we're done
        if not dnp:
            # See if it is in the deselected dictionary
            dnp = self.deselectedDict.get(id, None)
            if dnp:
                # It has been previously selected:
                # Show its bounding box
                dnp.highlight()
                # Remove it from the deselected dictionary
                del(self.deselectedDict[id])
            else:
                # Didn't find it, create a new selectedNodePath instance
                dnp = DirectNodePath(nodePath)
            # Add it to the selected dictionary
            self.selectedDict[dnp.id] = dnp
        # And update last
        self.last = dnp
        return dnp

    def deselect(self, nodePath):
        # Get this pointer
        id = nodePath.node().this
        # See if it is in the selected dictionary
        dnp = self.selectedDict.get(id, None)
        if dnp:
            # It was selected:
            # Hide its bounding box
            dnp.dehighlight()
            # Remove it from the selected dictionary
            del(self.selectedDict[id])
            # And keep track of it in the deselected dictionary
            self.deselectedDict[id] = dnp
        return dnp

    def selectedAsList(self):
        list = []
        for key in self.selectedDict.keys():
            list.append(self.selectedDict[key])

    def deselectedAsList(self):
        list = []
        for key in self.deselectedDict.keys():
            list.append(self.deselectedDict[key])

    def forEachSelectedNodePathDo(self, func):
        duplicateKeys = self.selectedDict.keys()[:]
        for key in duplicateKeys:
            func(self.selectedDict[key])

    def forEachDeselectedNodePathDo(self, func):
        duplicateKeys = self.deselectedDict.keys()[:]
        for key in duplicateKeys:
            func(self.deselectedDict[key])

    def deselectAll(self):
        self.forEachSelectedNodePathDo(self.deselect)

    def highlightAll(self):
        self.forEachSelectedNodePathDo(DirectNodePath.highlight)

    def dehighlightAll(self):
        self.forEachSelectedNodePathDo(DirectNodePath.dehighlight)

    def removeSelected(self):
	selected = self.dnp.last
        if selected:
            selected.remove()
        
    def removeAll(self):
	# Remove all selected nodePaths from the Scene Graph
        self.forEachSelectedNodePathDo(NodePath.remove)

    def toggleVizSelected(self):
	selected = self.dnp.last
        # Toggle visibility of selected node paths
        if selected:
            selected.toggleViz()

    def toggleVizAll(self):
        # Toggle viz for all selected node paths
        self.forEachSelectedNodePathDo(NodePath.toggleViz)

    def isolateSelected(self):
	selected = self.dnp.last
        if selected:
            selected.isolate()

    def getDirectNodePath(self, nodePath):
        # Get this pointer
        id = nodePath.node().this
        # First check selected dict
        dnp = self.selectedDict.get(id, None)
        if dnp:
            return dnp
        # Otherwise return result of deselected search
        return self.selectedDict.get(id, None)

    def getNumSelected(self):
        return len(self.selectedDict.keys())

"""
dd = loader.loadModel(r"I:\beta\toons\install\neighborhoods\donalds_dock")
dd.reparentTo(render)

# Operations on cq
cq.getNumEntries()
cq.getEntry(0).getIntoNode()
cq.getEntry(0).getIntoNode().getName()
print cq.getEntry(i).getIntoIntersectionPoint()

for i in range(0,bobo.cq.getNumEntries()):
    name = bobo.cq.getEntry(i).getIntoNode().getName()
    if not name:
        name = "<noname>"
    print name
    print bobo.cq.getEntry(i).getIntoIntersectionPoint()[0],
    print bobo.cq.getEntry(i).getIntoIntersectionPoint()[1],
    print bobo.cq.getEntry(i).getIntoIntersectionPoint()[2]
"""
