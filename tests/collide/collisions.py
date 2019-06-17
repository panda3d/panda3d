from panda3d.core import CollisionNode, NodePath
from panda3d.core import CollisionTraverser, CollisionHandlerQueue
from panda3d.core import CollisionSphere, CollisionInvSphere, CollisionBox, CollisionPolygon, CollisionCapsule
from panda3d.core import CollisionLine, CollisionRay, CollisionSegment, CollisionParabola
from panda3d.core import CollisionPlane
from panda3d.core import Point3, Vec3, Plane, LParabola


def make_collision(solid_from, solid_into):
    node_from = CollisionNode("from")
    node_from.add_solid(solid_from)
    node_into = CollisionNode("into")
    node_into.add_solid(solid_into)

    root = NodePath("root")
    trav = CollisionTraverser()
    queue = CollisionHandlerQueue()

    np_from = root.attach_new_node(node_from)
    np_into = root.attach_new_node(node_into)

    trav.add_collider(np_from, queue)
    trav.traverse(root)

    entry = None
    for e in queue.get_entries():
        if e.get_into() == solid_into:
            entry = e

    return (entry, np_from, np_into)
