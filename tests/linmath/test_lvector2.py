from math import floor, ceil
import sys

from panda3d.core import Vec2, Vec3, Vec4, Vec2F, Vec2D
import pytest


reason = '''Rounding in Python 2.7 expects to return a float value, since it returns a Vector it
does not work. When Python 2.7 gets deprecated, remove this check.'''


@pytest.mark.skipif(sys.version_info < (3, 5), reason=reason)
def test_round():
    original_vector = Vec2(2.3, -2.6)

    rounded_vector = round(original_vector)
    assert rounded_vector.x == 2
    assert rounded_vector.y == -3


@pytest.mark.skipif(sys.version_info < (3, 5), reason=reason)
def test_floor():
    original_vector = Vec2(2.3, -2.6)

    rounded_vector = floor(original_vector)
    assert rounded_vector.x == 2
    assert rounded_vector.y == -3


@pytest.mark.skipif(sys.version_info < (3, 5), reason=reason)
def test_ceil():
    original_vector = Vec2(2.3, -2.6)

    rounded_vector = ceil(original_vector)
    assert rounded_vector.x == 3
    assert rounded_vector.y == -2


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


def test_vec2_str():
    assert str(Vec2F(2, 3)) == "LVector2f(2, 3)"
    assert str(Vec2D(2, 3)) == "LVector2d(2, 3)"


def test_vec2_compare():
    assert Vec2(1, 2).compare_to(Vec2(1, 2)) == 0

    assert Vec2(1, 0).compare_to(Vec2(1, 0)) == 0
    assert Vec2(1, 0).compare_to(Vec2(0, 1)) == 1
    assert Vec2(0, 1).compare_to(Vec2(1, 0)) == -1
    assert Vec2(0, 1).compare_to(Vec2(0, 1)) == 0
