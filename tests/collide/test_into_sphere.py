from collisions import *


def test_sphere_into_sphere():
    sphere1 = CollisionSphere(0, 0, 3, 3)
    sphere2 = CollisionSphere(0, 0, 0, 3)

    entry, np_from, np_into = make_collision(sphere1, sphere2)
    assert entry is not None
    assert entry.get_from() == sphere1
    assert entry.get_into() == sphere2


def test_box_into_sphere():
    box = CollisionBox((0, 0, 0), 2, 3, 4)
    sphere = CollisionSphere(0, 0, 0, 3)

    entry, np_from, np_into = make_collision(box, sphere)
    assert entry is not None
    assert entry.get_from() == box
    assert entry.get_into() == sphere
