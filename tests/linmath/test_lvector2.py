from math import floor, ceil
import sys

from panda3d.core import Vec2, Vec3, Vec4, Vec2F, Vec2D
from panda3d import core
import pytest


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


def test_vec2_nan():
    nan = float("nan")
    inf = float("inf")
    assert not Vec2F(0, 0).is_nan()
    assert not Vec2F(1, 0).is_nan()
    assert Vec2F(nan, 0).is_nan()
    assert Vec2F(0, nan).is_nan()
    assert Vec2F(nan, nan).is_nan()
    assert Vec2F(-nan, 0).is_nan()
    assert Vec2F(-nan, nan).is_nan()
    assert Vec2F(inf, nan).is_nan()
    assert not Vec2F(inf, 0).is_nan()
    assert not Vec2F(inf, inf).is_nan()
    assert not Vec2F(-inf, 0).is_nan()

    assert not Vec2D(0, 0).is_nan()
    assert not Vec2D(1, 0).is_nan()
    assert Vec2D(nan, 0).is_nan()
    assert Vec2D(0, nan).is_nan()
    assert Vec2D(nan, nan).is_nan()
    assert Vec2D(-nan, 0).is_nan()
    assert Vec2D(-nan, nan).is_nan()
    assert Vec2D(inf, nan).is_nan()
    assert not Vec2D(inf, 0).is_nan()
    assert not Vec2D(inf, inf).is_nan()
    assert not Vec2D(-inf, 0).is_nan()


def test_vec2_round():
    original_vector = Vec2(2.3, -2.6)

    rounded_vector = round(original_vector)
    assert rounded_vector.x == 2
    assert rounded_vector.y == -3


def test_vec2_floor():
    original_vector = Vec2(2.3, -2.6)

    rounded_vector = floor(original_vector)
    assert rounded_vector.x == 2
    assert rounded_vector.y == -3


def test_vec2_ceil():
    original_vector = Vec2(2.3, -2.6)

    rounded_vector = ceil(original_vector)
    assert rounded_vector.x == 3
    assert rounded_vector.y == -2


def test_vec2_rmul():
    assert 2 * Vec2(3, -4) == Vec2(6, -8)


@pytest.mark.xfail(sys.platform == "win32", reason="unknown precision issue")
@pytest.mark.parametrize("type", (core.LVecBase2f, core.LVecBase2d, core.LVecBase2i))
def test_vec2_floordiv(type):
    with pytest.raises(ZeroDivisionError):
        type(1, 2) // 0

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


def test_vec2_repr():
    assert repr(Vec2F(0.1, 0.2)) == "LVector2f(0.1, 0.2)"
    assert repr(Vec2F(0.3, 0.4)) == "LVector2f(0.3, 0.4)"
    assert repr(Vec2F(-0.9999999403953552, 1.00000001)) == "LVector2f(-0.99999994, 1)"
    assert repr(Vec2F(0.00000001, 0.0)) == "LVector2f(1e-8, 0)"
    assert repr(Vec2D(0.1, 0.2)) == "LVector2d(0.1, 0.2)"
    assert repr(Vec2D(0.3, 0.4)) == "LVector2d(0.3, 0.4)"
    assert repr(Vec2D(-0.9999999403953552, 1.00000001)) == "LVector2d(-0.9999999403953552, 1.00000001)"
    assert repr(Vec2D(0.00000001, 0.0)) == "LVector2d(1e-8, 0)"


def test_vec2_buffer():
    v = Vec2(1.5, -10.0)
    m = memoryview(v)
    assert len(m) == 2
    assert m[0] == 1.5
    assert m[1] == -10.0
