import pytest

# Skip these tests if we can't import bullet.
bullet = pytest.importorskip("panda3d.bullet")
from panda3d import core


def reconstruct(object):
    # Create a temporary buffer, which we first write the object into, and
    # subsequently read it from again.
    buffer = core.DatagramBuffer()

    writer = core.BamWriter(buffer)
    writer.init()
    writer.write_object(object)

    reader = core.BamReader(buffer)
    reader.init()
    object = reader.read_object()
    reader.resolve()
    return object


def test_box_shape():
    shape = bullet.BulletBoxShape((1, 2, 3))
    shape.margin = 0.5

    shape2 = reconstruct(shape)

    assert type(shape) is type(shape2)
    assert shape.margin == shape2.margin
    assert shape.name == shape2.name
    assert shape.half_extents_without_margin == shape2.half_extents_without_margin
    assert shape.half_extents_with_margin == shape2.half_extents_with_margin


def test_capsule_shape():
    shape = bullet.BulletCapsuleShape(1.4, 3.5, bullet.Y_up)
    shape.margin = 0.5

    shape2 = reconstruct(shape)

    assert type(shape) is type(shape2)
    assert shape.margin == shape2.margin
    assert shape.name == shape2.name
    assert shape.radius == shape2.radius
    assert shape.height == shape2.height


def test_cone_shape():
    shape = bullet.BulletConeShape(1.4, 3.5, bullet.Y_up)
    shape.margin = 0.5

    shape2 = reconstruct(shape)

    assert type(shape) is type(shape2)
    assert shape.margin == shape2.margin
    assert shape.name == shape2.name
    assert shape.radius == shape2.radius
    assert shape.height == shape2.height


def test_cylinder_shape():
    shape = bullet.BulletCylinderShape(1.4, 3.5, bullet.Y_up)
    shape.margin = 0.5

    shape2 = reconstruct(shape)

    assert type(shape) is type(shape2)
    assert shape.margin == shape2.margin
    assert shape.name == shape2.name
    assert shape.radius == shape2.radius
    assert shape.half_extents_without_margin == shape2.half_extents_without_margin
    assert shape.half_extents_with_margin == shape2.half_extents_with_margin


def test_minkowski_sum_shape():
    box = bullet.BulletBoxShape((1, 2, 3))
    sphere = bullet.BulletSphereShape(2.0)

    shape = bullet.BulletMinkowskiSumShape(box, sphere)
    shape.transform_a = core.TransformState.make_pos((8, 7, 3))
    shape.transform_b = core.TransformState.make_hpr((45, -10, 110))
    shape.margin = 0.5

    shape2 = reconstruct(shape)

    assert type(shape) is type(shape2)
    assert shape.margin == shape2.margin
    assert shape.name == shape2.name
    assert shape.transform_a.mat.compare_to(shape2.transform_a.mat, 0.001) == 0
    assert shape.transform_b.mat.compare_to(shape2.transform_b.mat, 0.001) == 0
    assert type(shape.shape_a) == type(shape2.shape_a)
    assert type(shape.shape_b) == type(shape2.shape_b)


def test_multi_sphere_shape():
    shape = bullet.BulletMultiSphereShape([(1, 2, 3), (4, 5, 6)], [1.5, 2.5])
    shape.margin = 0.5

    shape2 = reconstruct(shape)

    assert type(shape) is type(shape2)
    assert shape.margin == shape2.margin
    assert shape.name == shape2.name
    assert shape.sphere_count == shape2.sphere_count
    assert tuple(shape.sphere_pos) == tuple(shape2.sphere_pos)
    assert tuple(shape.sphere_radius) == tuple(shape2.sphere_radius)


def test_plane_shape():
    shape = bullet.BulletPlaneShape((1.2, 0.2, 0.5), 2.5)
    shape.margin = 0.5

    shape2 = reconstruct(shape)

    assert type(shape) is type(shape2)
    assert shape.margin == shape2.margin
    assert shape.name == shape2.name
    assert shape.plane_normal.almost_equal(shape2.plane_normal, 0.1)
    assert shape.plane_constant == shape2.plane_constant


def test_sphere_shape():
    shape = bullet.BulletSphereShape(1.4)
    shape.margin = 0.5

    shape2 = reconstruct(shape)

    assert type(shape) is type(shape2)
    assert shape.margin == shape2.margin
    assert shape.name == shape2.name
    assert shape.radius == shape2.radius
