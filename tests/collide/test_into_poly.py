from collisions import *


def test_box_into_poly():
    box = CollisionBox((0, 0, 0), 2, 3, 4)
    poly = CollisionPolygon(Point3(0, 0, 0), Point3(0, 0, 1), Point3(0, 1, 1), Point3(0, 1, 0))

    entry = make_collision(box, poly)[0]
    assert entry is not None
    assert entry.get_from() == box
    assert entry.get_into() == poly

    # Colliding just on the edge
    entry, np_from, np_into = make_collision(CollisionBox((0, 3, 0), 1, 2, 1), poly)
    assert entry.get_surface_point(np_from) == Point3(0, 3, 0)
    assert entry.get_surface_normal(np_into) == Vec3(-1, 0, 0)  # Testing surface normal

    # No collision
    entry = make_collision(CollisionBox((10, 10, 10), 8, 9, 10), poly)[0]
    assert entry is None


def test_sphere_into_poly():
    sphere = CollisionSphere(0, 0, 0, 1)
    poly = CollisionPolygon(Point3(0, 0, 0), Point3(0, 0, 1), Point3(0, 1, 1), Point3(0, 1, 0))

    entry = make_collision(sphere, poly)[0]
    assert entry is not None
    assert entry.get_from() == sphere
    assert entry.get_into() == poly

    # Colliding just on the edge
    entry, np_from, np_into = make_collision(CollisionSphere(0, 0, 3, 2), poly)
    assert entry.get_surface_point(np_from) == Point3(0, 0, 3)
    assert entry.get_surface_normal(np_into) == Vec3(-1, 0, 0)  # Testing surface normal

    # No collision
    entry = make_collision(CollisionSphere(100, 100, 100, 100), poly)[0]
    assert entry is None


def test_plane_into_poly():
    # CollisionPlane is not a 'from' object
    plane = CollisionPlane(Plane(Vec3(0, 0, 1), Point3(0, 0, 0)))
    poly = CollisionPolygon(Point3(0, 0, 0), Point3(0, 0, 1), Point3(0, 1, 1), Point3(0, 1, 0))

    entry = make_collision(plane, poly)[0]
    assert entry is None
