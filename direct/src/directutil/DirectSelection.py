from PandaObject import *

class SelectionRay:
    def __init__(self, camera, fGeom = 1):
        # Record the camera associated with this selection ray
        self.camera = camera
        # Create a collision node
        self.rayCollisionNodePath = camera.attachNewNode( CollisionNode() )
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

    def localHitPt(self, index):
        return self.cq.getEntry(index).getIntoIntersectionPoint()

    def hitPt(self, index):
        entry = self.cq.getEntry(index)
        hitPt = entry.getIntoIntersectionPoint()
        return entry.getWrtSpace().xformPoint(hitPt)

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
