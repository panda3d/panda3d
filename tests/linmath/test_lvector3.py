from math import floor, ceil
import sys

from panda3d.core import Vec2, Vec3, Vec3F, Vec3D
from panda3d import core
import pytest


def test_vec3_creation():
    assert Vec3(x=1, y=2, z=1) == Vec3(1, 2, 1) == Vec3((1, 2, 1))


def test_vec3_getter_setter():
    original_vector = Vec3(2, 3, 7)

    assert original_vector.x == 2
    assert original_vector.y == 3
    assert original_vector.z == 7

    original_vector.x = 1
    original_vector.y = 3
    original_vector.z = 5

    assert original_vector == Vec3(1, 3, 5)

    original_vector[0] = 3
    original_vector[1] = 1
    original_vector[2] = 1

    assert original_vector == Vec3(3, 1, 1)

    original_vector.set_x(-8)
    original_vector.set_y(6)
    original_vector.set_z(10)

    assert original_vector.x == -8
    assert original_vector.y == 6
    assert original_vector.z == 10


def test_vec3_sum():
    original_vector = Vec3(2, 3, -2)

    assert original_vector + original_vector == Vec3(4, 6, -4)
    assert original_vector + 3 == Vec3(5, 6, 1)


def test_vec3_power():
    assert Vec3(2, -3, 2) ** 2 == Vec3(4, 9, 4)


def test_vec3_len():
    assert len(Vec3(2, -3, 10)) == 3


def test_vec3_swizzle_mask():
    original_vector = Vec3(3, 5, 1)

    assert original_vector.xy == Vec2(3, 5)
    assert original_vector.zxy == Vec3(1, 3, 5)


def test_vec3_str():
    assert str(Vec3F(2, 3, 1)) == "LVector3f(2, 3, 1)"
    assert str(Vec3D(2, 3, 1)) == "LVector3d(2, 3, 1)"


def test_vec3_compare():
    assert Vec3(1, 2, 3).compare_to(Vec3(1, 2, 3)) == 0

    assert Vec3(1, 0, 0).compare_to(Vec3(1, 0, 0)) == 0
    assert Vec3(1, 0, 0).compare_to(Vec3(0, 1, 0)) == 1
    assert Vec3(1, 0, 0).compare_to(Vec3(0, 0, 1)) == 1
    assert Vec3(0, 1, 0).compare_to(Vec3(1, 0, 0)) == -1
    assert Vec3(0, 1, 0).compare_to(Vec3(0, 1, 0)) == 0
    assert Vec3(0, 1, 0).compare_to(Vec3(0, 0, 1)) == 1
    assert Vec3(0, 0, 1).compare_to(Vec3(1, 0, 0)) == -1
    assert Vec3(0, 0, 1).compare_to(Vec3(0, 1, 0)) == -1
    assert Vec3(0, 0, 1).compare_to(Vec3(0, 0, 1)) == 0


def test_vec3_round():
    original_vector = Vec3(2.3, -2.6, 3.5)

    rounded_vector = round(original_vector)
    assert rounded_vector.x == 2
    assert rounded_vector.y == -3
    assert rounded_vector.z == 4


def test_vec3_floor():
    original_vector = Vec3(2.3, -2.6, 3.5)

    rounded_vector = floor(original_vector)
    assert rounded_vector.x == 2
    assert rounded_vector.y == -3
    assert rounded_vector.z == 3


def test_vec3_ceil():
    original_vector = Vec3(2.3, -2.6, 3.5)

    rounded_vector = ceil(original_vector)
    assert rounded_vector.x == 3
    assert rounded_vector.y == -2
    assert rounded_vector.z == 4


def test_vec3_rmul():
    assert 2 * Vec3(0, 3, -4) == Vec3(0, 6, -8)


@pytest.mark.xfail(sys.platform == "win32", reason="unknown precision issue")
@pytest.mark.parametrize("type", (core.LVecBase3f, core.LVecBase3d, core.LVecBase3i))
def test_vec3_floordiv(type):
    with pytest.raises(ZeroDivisionError):
        type(1, 2, 3) // 0

    for i in range(-11, 11):
        for j in range(1, 11):
            assert (type(i) // j).x == i // j
            assert (type(i) // -j).x == i // -j

            v = type(i)
            v //= j
            assert v.x == i // j

            v = type(i)
            v //= -j
            assert v.x == i // -j


def test_vec3_repr():
    assert repr(Vec3F(0.1, 0.2, 0.3)) == "LVector3f(0.1, 0.2, 0.3)"
    assert repr(Vec3F(-0.9999999403953552, 1.00000001, 1)) == "LVector3f(-0.99999994, 1, 1)"
    assert repr(Vec3F(-9.451235e29, 9.451234e-19, 1e-11)) == "LVector3f(-9.451235e29, 9.451234e-19, 1e-11)"
    assert repr(Vec3F(0.001, 0.0001, 0.00001)) == "LVector3f(0.001, 0.0001, 0.00001)"
    assert repr(Vec3F(-0.001, -0.0001, -0.00001)) == "LVector3f(-0.001, -0.0001, -0.00001)"
    assert repr(Vec3D(0.1, 0.2, 0.3)) == "LVector3d(0.1, 0.2, 0.3)"
    assert repr(Vec3D(-0.9999999403953552, 1.00000001, 1)) == "LVector3d(-0.9999999403953552, 1.00000001, 1)"
    assert repr(Vec3D(-9.451235e29, 9.451234e-19, 1e-11)) == "LVector3d(-9.451235e29, 9.451234e-19, 1e-11)"
    assert repr(Vec3D(0.001, 0.0001, 0.00001)) == "LVector3d(0.001, 0.0001, 0.00001)"
    assert repr(Vec3D(-0.001, -0.0001, -0.00001)) == "LVector3d(-0.001, -0.0001, -0.00001)"


def test_vec3_buffer():
    v = Vec3(0.5, 2.0, -10.0)
    m = memoryview(v)
    assert len(m) == 3
    assert m[0] == 0.5
    assert m[1] == 2.0
    assert m[2] == -10.0
