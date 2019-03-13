from panda3d.core import CollisionNode, NodePath
from panda3d.core import CollisionTraverser, CollisionHandlerQueue
from panda3d.core import CollisionSphere, CollisionBox, CollisionPolygon
from panda3d.core import Point3


def make_sphere_node(name, cx, cy, cz, radius):
    sphere = CollisionSphere(cx, cy, cz, radius)
    node = CollisionNode(name)
    node.add_solid(sphere)
    return node


def make_box_node(name, center, dx, dy, dz):
    box = CollisionBox(center, dx, dy, dz)
    node = CollisionNode(name)
    node.add_solid(box)
    return node


def make_polygon_node(name, point1, point2, point3, point4=None):
    poly = CollisionPolygon(point1, point2, point3, point4)
    node = CollisionNode(name)
    node.add_solid(poly)
    return node


def get_into_count(node1, node2):
    root = NodePath("root")
    trav = CollisionTraverser()
    queue = CollisionHandlerQueue()
    np = root.attach_new_node(node1)
    np2 = root.attach_new_node(node2)
    trav.add_collider(np, queue)
    trav.traverse(root)

    into_count = 0
    for i in range(queue.get_num_entries()):
        if queue.get_entry(i).get_into_node_path() == np2:
            into_count += 1

    return into_count


def test_sphere_into_sphere():
    node1 = make_sphere_node("sphere1", 0, 0, 3, 3)
    node2 = make_sphere_node("sphere2", 0, 0, 0, 3)
    into_count = get_into_count(node1, node2)
    assert into_count > 0


def test_sphere_into_box():
    node1 = make_sphere_node("sphere", 0, 0, 0, 3)
    node2 = make_box_node("box", (0, 0, 0), 2, 3, 4)
    into_count = get_into_count(node1, node2)
    assert into_count > 0


def test_box_into_sphere():
    node1 = make_box_node("box", (0, 0, 0), 2, 3, 4)
    node2 = make_sphere_node("sphere", 0, 0, 0, 3)
    into_count = get_into_count(node1, node2)
    assert into_count > 0


def test_box_into_poly():
    node1 = make_box_node("box", (0, 0, 0), 2, 3, 4)
    node2 = make_polygon_node("polygon", Point3(0, 0, 0), Point3(0, 0, 1), Point3(0, 1, 1), Point3(0, 1, 0))
    into_count = get_into_count(node1, node2)
    assert into_count > 0


def test_sphere_into_poly():
    node1 = make_sphere_node("sphere", 0, 0, 0, 1)
    node2 = make_polygon_node("polygon", Point3(0, 0, 0), Point3(0, 0, 1), Point3(0, 1, 1), Point3(0, 1, 0))
    into_count = get_into_count(node1, node2)
    assert into_count > 0
