from panda3d.core import CollisionNode, NodePath
from panda3d.core import CollisionTraverser, CollisionHandlerQueue
from panda3d.core import CollisionSphere, CollisionBox, CollisionPolygon
from panda3d.core import Point3


def make_sphere(name, cx, cy, cz, radius):
    sphere = CollisionSphere(cx, cy, cz, radius)
    node = CollisionNode(name)
    node.add_solid(sphere)
    return (sphere, node)


def make_box(name, center, dx, dy, dz):
    box = CollisionBox(center, dx, dy, dz)
    node = CollisionNode(name)
    node.add_solid(box)
    return (box, node)


def make_polygon(name, point1, point2, point3, point4=None):
    poly = CollisionPolygon(point1, point2, point3, point4)
    node = CollisionNode(name)
    node.add_solid(poly)
    return (poly, node)


def make_collision(node1, node2):
    root = NodePath("root")
    trav = CollisionTraverser()
    queue = CollisionHandlerQueue()
    np = root.attach_new_node(node1)
    np2 = root.attach_new_node(node2)
    trav.add_collider(np, queue)
    trav.traverse(root)

    entry = None
    for i in range(queue.get_num_entries()):
        if queue.get_entry(i).get_into_node_path() == np2:
            entry = queue.get_entry(i)

    return (entry, np, np2)
