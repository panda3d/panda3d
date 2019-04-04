from collisions import *


def test_sphere_into_box():
    sphere = CollisionSphere(0, 0, 4, 3)
    box = CollisionBox((0, 0, 0), 2, 3, 4)
    entry = make_collision(sphere, box)[0]
    assert entry is not None
    assert entry.get_from() == sphere
    assert entry.get_into() == box

    # Colliding just on the edge
    entry, np_from, np_into = make_collision(CollisionSphere(0, 0, 10, 6), box)
    assert entry.get_surface_point(np_from) == Point3(0, 0, 4)
    assert entry.get_surface_normal(np_into) == Vec3(0, 0, 1)  # Testing surface normal

    # No collision
    entry = make_collision(CollisionSphere(100, 100, 100, 100), box)[0]
    assert entry is None

def test_plane_into_sphere():
    # CollisionPlane is not a 'from' object
    plane = CollisionPlane(Plane(Vec3(0, 0, 1), Point3(0, 0, 0)))
    box = CollisionBox((0, 0, 0), 2, 3, 4)

    entry = make_collision(plane, box)[0]
    assert entry is None
