from collisions import *


def test_sphere_into_box():
    sphere = CollisionSphere(0, 0, 0, 3)
    box = CollisionBox((0, 0, 0), 2, 3, 4)

    entry, np_from, np_into = make_collision(sphere, box)
    assert entry is not None
    assert entry.get_from() == sphere
    assert entry.get_into() == box
