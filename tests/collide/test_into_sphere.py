from collisions import *


def test_sphere_into_sphere():
    sphere1 = CollisionSphere(0, 0, 3, 3)
    sphere2 = CollisionSphere(0, 0, 0, 3)

    entry = make_collision(sphere1, sphere2)[0]
    assert entry is not None
    assert entry.get_from() == sphere1
    assert entry.get_into() == sphere2

    # Colliding just on the edge
    entry, np_from, np_into = make_collision(CollisionSphere(0, 0, 10, 7), sphere2)
    assert entry.get_surface_point(np_from) == Point3(0, 0, 3)

    # No collision
    entry = make_collision(CollisionSphere(0, 0, 10, 6), sphere2)[0]
    assert entry is None


def test_box_into_sphere():
    box = CollisionBox((0, 0, 0), 2, 3, 4)
    sphere = CollisionSphere(0, 0, 0, 3)

    entry = make_collision(box, sphere)[0]
    assert entry is not None
    assert entry.get_from() == box
    assert entry.get_into() == sphere

    # Colliding just on the edge
    entry, np_from, np_into = make_collision(CollisionBox((0, 0, 10), 6, 6, 7), sphere)
    assert entry.get_surface_point(np_from) == Point3(0, 0, 3)

    # No collision
    entry = make_collision(CollisionBox((0, 0, 10), 6, 6, 6), sphere)[0]
    assert entry is None
