from panda3d.core import Plane, BoundingPlane, BoundingSphere, BoundingVolume


def test_plane_contains_sphere():
    plane = BoundingPlane((0, 0, 1, 0))

    # Sphere above plane
    assert plane.contains(BoundingSphere((0, 0, 2), 1)) == BoundingVolume.IF_no_intersection

    # Sphere intersecting surface of plane
    assert plane.contains(BoundingSphere((0, 0, 0), 1)) == BoundingVolume.IF_possible | BoundingVolume.IF_some

    # Sphere below plane
    assert plane.contains(BoundingSphere((0, 0, -2), 1)) == BoundingVolume.IF_possible | BoundingVolume.IF_some | BoundingVolume.IF_all


def test_plane_contains_plane():
    # Plane should always fully contain itself.
    a = BoundingPlane((1, 0, 0, 1))
    assert a.contains(a) == BoundingVolume.IF_possible | BoundingVolume.IF_some | BoundingVolume.IF_all

    # Plane with its mirror image
    a = BoundingPlane((1, 0, 0, 1))
    b = BoundingPlane((-1, 0, 0, -1))
    assert a.contains(b) == BoundingVolume.IF_no_intersection
    assert b.contains(a) == BoundingVolume.IF_no_intersection

    # One plane above the other
    a = BoundingPlane(Plane((1, 0, 0), (1, 0, 0)))
    b = BoundingPlane(Plane((1, 0, 0), (2, 0, 0)))
    assert a.contains(b) == BoundingVolume.IF_possible | BoundingVolume.IF_some
    assert b.contains(a) == BoundingVolume.IF_possible | BoundingVolume.IF_some | BoundingVolume.IF_all

    # Opposing planes with distance between them.
    a = BoundingPlane(Plane((1, 0, 0), (1, 0, 0)))
    b = BoundingPlane(Plane((-1, 0, 0), (2, 0, 0)))
    assert a.contains(b) == BoundingVolume.IF_no_intersection
    assert b.contains(a) == BoundingVolume.IF_no_intersection

    # Planes overlapping in the same axis.
    a = BoundingPlane(Plane((1, 0, 0), (2, 0, 0)))
    b = BoundingPlane(Plane((-1, 0, 0), (1, 0, 0)))
    assert a.contains(b) == BoundingVolume.IF_possible | BoundingVolume.IF_some
    assert b.contains(a) == BoundingVolume.IF_possible | BoundingVolume.IF_some

    # Planes overlapping due to not sharing a normal vector.
    a = BoundingPlane(Plane((1, 0, 0), (2, 0, 0)))
    b = BoundingPlane(Plane((0.8, 0.6, 0), (4, 0, 0)))
    assert a.contains(b) == BoundingVolume.IF_possible | BoundingVolume.IF_some
    assert b.contains(a) == BoundingVolume.IF_possible | BoundingVolume.IF_some

    # Same as above.
    a = BoundingPlane(Plane((1, 0, 0), (2, 0, 0)))
    b = BoundingPlane(Plane((-0.8, -0.6, 0), (4, 0, 0)))
    assert a.contains(b) == BoundingVolume.IF_possible | BoundingVolume.IF_some
    assert b.contains(a) == BoundingVolume.IF_possible | BoundingVolume.IF_some

    # Planes pointing along different major axes.
    a = BoundingPlane(Plane((1, 0, 0, 0)))
    b = BoundingPlane(Plane((0, 1, 0, 0)))
    c = BoundingPlane(Plane((0, 0, 1, 0)))
    assert a.contains(b) == BoundingVolume.IF_possible | BoundingVolume.IF_some
    assert b.contains(c) == BoundingVolume.IF_possible | BoundingVolume.IF_some
    assert a.contains(c) == BoundingVolume.IF_possible | BoundingVolume.IF_some
