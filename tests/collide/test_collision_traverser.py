from panda3d.core import CollisionTraverser, CollisionHandlerQueue
from panda3d.core import NodePath, CollisionNode



def test_collision_traverser_pickle():
    from direct.stdpy.pickle import dumps, loads

    handler = CollisionHandlerQueue()

    collider1 = NodePath(CollisionNode("collider1"))
    collider2 = NodePath(CollisionNode("collider2"))

    trav = CollisionTraverser("test123")
    trav.respect_prev_transform = True
    trav.add_collider(collider1, handler)
    trav.add_collider(collider2, handler)

    trav = loads(dumps(trav, -1))
    assert trav.respect_prev_transform is True

    assert trav.name == "test123"
    assert trav.get_num_colliders() == 2
    collider1 = trav.get_collider(0)
    collider2 = trav.get_collider(1)
    assert collider1.name == "collider1"
    assert collider2.name == "collider2"

    # Two colliders must still be the same object; this only works with our own
    # version of the pickle module, in direct.stdpy.pickle.
    assert trav.get_handler(collider1) == trav.get_handler(collider2)
