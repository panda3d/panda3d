from collisions import *


def test_box_into_poly():
    box = make_box("box", (0, 0, 0), 2, 3, 4)
    poly = make_polygon("polygon", Point3(0, 0, 0), Point3(0, 0, 1), Point3(0, 1, 1), Point3(0, 1, 0))
    node1 = box[1]
    node2 = poly[1]

    entry, np, np2 = make_collision(node1, node2)
    assert entry is not None
    assert entry.get_from() == box[0]
    assert entry.get_into() == poly[0]


def test_sphere_into_poly():
    sphere = make_sphere("sphere", 0, 0, 0, 1)
    poly = make_polygon("polygon", Point3(0, 0, 0), Point3(0, 0, 1), Point3(0, 1, 1), Point3(0, 1, 0))
    node1 = sphere[1]
    node2 = poly[1]

    entry, np, np2 = make_collision(node1, node2)
    assert entry is not None
    assert entry.get_from() == sphere[0]
    assert entry.get_into() == poly[0]
