import sys

import pytest

from panda3d.core import Vec2, Vec3, Vec4


def test_vec2_creation():
    assert Vec2(x=1, y=2) == Vec2(1, 2) == Vec2((1, 2))


def test_vec2_getter_setter():
    original_vector = Vec2(2, 3)

    assert original_vector.x == 2
    assert original_vector.y == 3

    original_vector.x = 1
    original_vector.y = 3

    assert original_vector == Vec2(1, 3)

    original_vector[0] = 3
    original_vector[1] = 1

    assert original_vector == Vec2(3, 1)

    original_vector.set_x(-8)
    original_vector.set_y(6)

    assert original_vector.x == -8
    assert original_vector.y == 6


def test_vec2_sum():
    original_vector = Vec2(2, 3)

    assert original_vector + original_vector == Vec2(4, 6)
    assert original_vector + 3 == Vec2(5, 6)


def test_vec2_power():
    assert Vec2(2, -3) ** 2 == Vec2(4, 9)


def test_vec2_len():
    assert len(Vec2(2, -3)) == 2


def test_vec2_swizzle_mask():
    original_vector = Vec2(3, 5)

    assert original_vector.yx == Vec2(5, 3)
    assert original_vector.xy == original_vector


def test_vec2_repr():
    assert str(Vec2(2, 3)) == "LVector2f(2, 3)"
