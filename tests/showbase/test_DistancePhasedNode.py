from panda3d import core
from direct.showbase.DistancePhasedNode import BufferedDistancePhasedNode


def test_BufferedDistancePhasedNode(base):
    cSphere = core.CollisionSphere(0, 0, 0, 0.1)
    cNode = core.CollisionNode('camCol')
    cNode.addSolid(cSphere)
    cNodePath = core.NodePath(cNode)
    # cNodePath.reparentTo(base.cam)
    # cNodePath.show()
    # cNodePath.setPos(25,0,0)

    base.cTrav = core.CollisionTraverser()

    eventHandler = core.CollisionHandlerEvent()
    eventHandler.addInPattern('enter%in')
    eventHandler.addOutPattern('exit%in')

    # messenger.toggleVerbose()
    base.cTrav.addCollider(cNodePath, eventHandler)

    p = BufferedDistancePhasedNode('p', {'At': (10, 20), 'Near': (100, 200), 'Far': (1000, 1020)},
                                   autoCleanup=False,
                                   fromCollideNode=cNodePath,
                                   )

    p.reparentTo(base.render)
    p._DistancePhasedNode__oneTimeCollide()
    base.eventMgr.doEvents()
