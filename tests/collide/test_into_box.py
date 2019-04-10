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


def test_plane_into_box():
    # CollisionPlane is not a 'from' object
    plane = CollisionPlane(Plane(Vec3(0, 0, 1), Point3(0, 0, 0)))
    box = CollisionBox((0, 0, 0), 2, 3, 4)

    entry = make_collision(plane, box)[0]
    assert entry is None


def test_ray_into_box():
    ray = CollisionRay(1, 1, 1, 0, 1, 0)
    box = CollisionBox((0, 0, 0), 3, 3, 5)
    entry = make_collision(ray, box)[0]
    assert entry is not None
    assert entry.get_from() == ray
    assert entry.get_into() == box

    # Colliding just on the edge
    entry, np_from, np_into = make_collision(CollisionRay(3, 3, 0, 1, -1, 0), box)
    assert entry.get_surface_point(np_from) == Point3(3, 3, 0)

    # No collision
    entry = make_collision(CollisionRay(0, 0, 100, 1, 0, 0), box)[0]
    assert entry is None
