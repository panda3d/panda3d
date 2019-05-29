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


def test_parabola_into_box():
    # Set up Parabola and Box
    parabola = CollisionParabola()
    parabola.set_t1(0)
    parabola.set_t2(2)
    box = CollisionBox((0,0,0), 3, 3, 3)

    # Parabola is inside the Box and not colliding
    parabola.set_parabola(
            LParabola((-1, 0, -1), (1, 0, 1), (1, 1, 1)))
    entry = make_collision(parabola, box)[0]
    assert entry is None

    # Parabola is inside the Box and colliding on its projectile
    parabola.set_parabola(
            LParabola((0, 0, 1), (0, 0, 1), (1, 1, 1)))
    # Parabola collides with Box when t == 1 at point (1, 1, 3)
    assert parabola.get_parabola().calc_point(1)  == (1, 1, 3)
    entry, np_from, into = make_collision(parabola, box)
    assert entry.get_surface_point(np_from) == (1, 1, 3)
    assert entry.get_from() == parabola
    assert entry.get_into() == box

    # Parabola is inside the Box and colliding on one of the endpoints
    parabola.set_parabola(
            LParabola((0, 0, 0), (0, 0, 1), (-3, 0, -3)))
    entry, np_from, np_into = make_collision(parabola, box)
    assert entry.get_surface_point(np_from) == (-3, 0, -3)

    # Parabola is outside the Box and not colliding
    parabola.set_parabola(
            LParabola((0, 0, 0), (0, 0, 1), (-5, 0, 0)))
    entry = make_collision(parabola, box)[0]
    assert entry is None

    # Parabola is outside the Box and colliding on its projecticle
    parabola.set_parabola(
            LParabola((-2, -2, -2), (1, 1, 1), (4, 4, 4)))
    # Parabola collides with Box when t == 1 at point (3, 3, 3)
    assert parabola.get_parabola().calc_point(1) == (3, 3, 3)
    entry, np_from, into = make_collision(parabola, box)
    assert entry.get_surface_point(np_from) == (3, 3, 3)

    # Parabola is outside the Box and colliding on the first endpoint
    parabola.set_parabola(
            LParabola((1, 1, 1), (1, 1, 1), (3, 3, 3)))
    entry, np_from, np_into = make_collision(parabola, box)
    assert entry.get_surface_point(np_from) == (3, 3, 3)

    # Parabola is outside the Box and colliding on the second endpoint
    parabola.set_parabola(
            LParabola((1, 0, 1), (-1, 0, -1), (-5, -3, -5)))
    assert parabola.get_parabola().calc_point(2) == (-3, -3, -3)
    entry, np_from, np_into = make_collision(parabola, box)
    assert entry.get_surface_point(np_from) == (-3, -3, -3)

    # Parabola intersects the Box at two points,
    # t == 0 and t == 2 the earliest one should be chosen.
    parabola.set_parabola(
            LParabola((-1, -1, -1), (-1, -1, -1), (3, 3, 3)))
    entry, np_from, np_into = make_collision(parabola, box)
    assert entry.get_surface_point(np_from) == parabola.get_parabola().calc_point(0)

    # First point no longer intersecting
    parabola.set_t1(1)
    entry, np_from, np_into = make_collision(parabola, box)
    assert parabola.get_parabola().calc_point(2) == (-3, -3, -3)
    assert entry.get_surface_point(np_from) == parabola.get_parabola().calc_point(2)
    assert entry.get_surface_normal(np_from) is not None
