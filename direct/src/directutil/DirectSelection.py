dd = loader.loadModel(r"I:\beta\toons\install\neighborhoods\donalds_dock")
dd.reparentTo(render)

cam = base.cam.node()
rNode = camera.attachNewNode( CollisionNode() )
rNode.node().setCollideGeom(1)
ray = CollisionRay()
rNode.node().addSolid( ray )
cq = CollisionHandlerQueue()
ct = CollisionTraverser( )
# Optionally
# ct = CollisionTraverser( RenderRelation.getClassType() )
ct.addCollider(rNode.node(), cq )
# These are the mouse coordinates
ray.setProjection( cam, 0, 0 )
ct.traverse( render.node() )

# Operations on cq
cq.getNumEntries()
cq.getEntry(0).getIntoNode()
cq.getEntry(0).getIntoNode().getName()
print cq.getEntry(i).getIntoIntersectionPoint()

for i in range(0,cq.getNumEntries()):
    name = cq.getEntry(i).getIntoNode().getName()
    if not name:
        name = "<noname>"
    print name
    print cq.getEntry(i).getIntoIntersectionPoint()[0],
    print cq.getEntry(i).getIntoIntersectionPoint()[1],
    print cq.getEntry(i).getIntoIntersectionPoint()[2]
