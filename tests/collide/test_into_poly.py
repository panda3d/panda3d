from collisions import *


def test_box_into_poly():
    box = CollisionBox((0, 0, 0), 2, 3, 4)
    poly = CollisionPolygon(Point3(0, 0, 0), Point3(0, 0, 1), Point3(0, 1, 1), Point3(0, 1, 0))

    entry, np_from, np_into =  make_collision(box, poly)
    assert entry is not None
    assert entry.get_from() == box
    assert entry.get_into() == poly


def test_sphere_into_poly():
    sphere = CollisionSphere(0, 0, 0, 1)
    poly = CollisionPolygon(Point3(0, 0, 0), Point3(0, 0, 1), Point3(0, 1, 1), Point3(0, 1, 0))

    entry, np_from, np_into = make_collision(sphere, poly)
    assert entry is not None
    assert entry.get_from() == sphere
    assert entry.get_into() == poly
