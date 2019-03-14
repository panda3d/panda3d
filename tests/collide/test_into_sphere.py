from collisions import *


def test_sphere_into_sphere():
    sphere1 = make_sphere("sphere1", 0, 0, 3, 3)
    sphere2 = make_sphere("sphere2", 0, 0, 0, 3)
    node1 = sphere1[1]
    node2 = sphere2[1]

    entry, np, np2 = make_collision(node1, node2)
    assert entry is not None
    assert entry.get_from() == sphere1[0]
    assert entry.get_into() == sphere2[0]


def test_box_into_sphere():
    box = make_box("box", (0, 0, 0), 2, 3, 4)
    sphere = make_sphere("sphere", 0, 0, 0, 3)
    node1 = box[1]
    node2 = sphere[1]

    entry, np, np2 = make_collision(node1, node2)
    assert entry is not None
    assert entry.get_from() == box[0]
    assert entry.get_into() == sphere[0]
