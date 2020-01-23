import sys

import pytest

from panda3d.core import Vec2, Vec3


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


def test_vec3_repr():
    assert str(Vec3(2, 3, 1)) == "LVector3f(2, 3, 1)"

