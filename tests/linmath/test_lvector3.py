from math import floor, ceil
import sys

from panda3d.core import Vec2, Vec3, Vec3F, Vec3D
import pytest


reason = '''Rounding in Python 2.7 expects to return a float value, since it returns a Vector it
does not work. When Python 2.7 gets deprecated, remove this check.'''

@pytest.mark.skipif(sys.version_info < (3, 5), reason=reason)
def test_round():
    original_vector = Vec3(2.3, -2.6, 3.5)

    rounded_vector = round(original_vector)
    assert rounded_vector.x == 2
    assert rounded_vector.y == -3
    assert rounded_vector.z == 4


@pytest.mark.skipif(sys.version_info < (3, 5), reason=reason)
def test_floor():
    original_vector = Vec3(2.3, -2.6, 3.5)

    rounded_vector = floor(original_vector)
    assert rounded_vector.x == 2
    assert rounded_vector.y == -3
    assert rounded_vector.z == 3


@pytest.mark.skipif(sys.version_info < (3, 5), reason=reason)
def test_ceil():
    original_vector = Vec3(2.3, -2.6, 3.5)

    rounded_vector = ceil(original_vector)
    assert rounded_vector.x == 3
    assert rounded_vector.y == -2
    assert rounded_vector.z == 4


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
