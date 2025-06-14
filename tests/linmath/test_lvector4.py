from math import floor, ceil
import sys

from panda3d.core import Vec2, Vec3, Vec4, Vec4F, Vec4D
from panda3d import core
import pytest


def test_vec4_creation():
    assert Vec4(x=1, y=2, z=1, w=7) == Vec4(1, 2, 1, 7) == Vec4((1, 2, 1, 7))


def test_vec4_getter_setter():
    original_vector = Vec4(2, 3, 7, 9)

    assert original_vector.x == 2
    assert original_vector.y == 3
    assert original_vector.z == 7
    assert original_vector.w == 9

    original_vector.x = 1
    original_vector.y = 3
    original_vector.z = 5
    original_vector.w = -8

    assert original_vector == Vec4(1, 3, 5, -8)

    original_vector[0] = 3
    original_vector[1] = 1
    original_vector[2] = 1
    original_vector[3] = -2

    assert original_vector == Vec4(3, 1, 1, -2)

    original_vector.set_x(-8)
    original_vector.set_y(6)
    original_vector.set_z(10)
    original_vector.set_w(30)

    assert original_vector.x == -8
    assert original_vector.y == 6
    assert original_vector.z == 10
    assert original_vector.w == 30


def test_vec4_sum():
    original_vector = Vec4(2, 3, -2, 1)

    assert original_vector + original_vector == Vec4(4, 6, -4, 2)
    assert original_vector + 3 == Vec4(5, 6, 1, 4)


def test_vec4_power():
    assert Vec4(2, -3, 2, -1) ** 2 == Vec4(4, 9, 4, 1)


def test_vec4_len():
    assert len(Vec4(2, -3, 10, 30)) == 4


def test_vec4_swizzle_mask():
    original_vector = Vec4(3, 5, 1, 0)

    assert original_vector.xy == Vec2(3, 5)
    assert original_vector.zxy == Vec3(1, 3, 5)
    assert original_vector.zxyw == Vec4(1, 3, 5, 0)


def test_vec4_str():
    assert str(Vec4F(2, 3, 1, 9)) == "LVector4f(2, 3, 1, 9)"
    assert str(Vec4D(2, 3, 1, 9)) == "LVector4d(2, 3, 1, 9)"


def test_vec4_compare():
    assert Vec4(1, 2, 3, 4).compare_to(Vec4(1, 2, 3, 4)) == 0

    assert Vec4(1, 0, 0, 0).compare_to(Vec4(1, 0, 0, 0)) == 0
    assert Vec4(1, 0, 0, 0).compare_to(Vec4(0, 1, 0, 0)) == 1
    assert Vec4(1, 0, 0, 0).compare_to(Vec4(0, 0, 1, 0)) == 1
    assert Vec4(1, 0, 0, 0).compare_to(Vec4(0, 0, 0, 1)) == 1
    assert Vec4(0, 1, 0, 0).compare_to(Vec4(1, 0, 0, 0)) == -1
    assert Vec4(0, 1, 0, 0).compare_to(Vec4(0, 1, 0, 0)) == 0
    assert Vec4(0, 1, 0, 0).compare_to(Vec4(0, 0, 1, 0)) == 1
    assert Vec4(0, 1, 0, 0).compare_to(Vec4(0, 0, 0, 1)) == 1
    assert Vec4(0, 0, 1, 0).compare_to(Vec4(1, 0, 0, 0)) == -1
    assert Vec4(0, 0, 1, 0).compare_to(Vec4(0, 1, 0, 0)) == -1
    assert Vec4(0, 0, 1, 0).compare_to(Vec4(0, 0, 1, 0)) == 0
    assert Vec4(0, 0, 1, 0).compare_to(Vec4(0, 0, 0, 1)) == 1
    assert Vec4(0, 0, 0, 1).compare_to(Vec4(1, 0, 0, 0)) == -1
    assert Vec4(0, 0, 0, 1).compare_to(Vec4(0, 1, 0, 0)) == -1
    assert Vec4(0, 0, 0, 1).compare_to(Vec4(0, 0, 1, 0)) == -1
    assert Vec4(0, 0, 0, 1).compare_to(Vec4(0, 0, 0, 1)) == 0


def test_vec4_round():
    original_vector = Vec4(2.3, -2.6, 3.5, 1)

    rounded_vector = round(original_vector)
    assert rounded_vector.x == 2
    assert rounded_vector.y == -3
    assert rounded_vector.z == 4
    assert rounded_vector.w == 1


def test_vec4_floor():
    original_vector = Vec4(2.3, -2.6, 3.5, 1)

    rounded_vector = floor(original_vector)
    assert rounded_vector.x == 2
    assert rounded_vector.y == -3
    assert rounded_vector.z == 3
    assert rounded_vector.w == 1


def test_vec4_ceil():
    original_vector = Vec4(2.3, -2.6, 3.5, 1)

    rounded_vector = ceil(original_vector)
    assert rounded_vector.x == 3
    assert rounded_vector.y == -2
    assert rounded_vector.z == 4
    assert rounded_vector.w == 1


def test_vec4_rmul():
    assert 2 * Vec4(0, 3, -4, 0.5) == Vec4(0, 6, -8, 1)


@pytest.mark.xfail(sys.platform == "win32", reason="unknown precision issue")
@pytest.mark.parametrize("type", (core.LVecBase4f, core.LVecBase4d, core.LVecBase4i))
def test_vec4_floordiv(type):
    with pytest.raises(ZeroDivisionError):
        type(1, 2, 3, 4) // 0

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


def test_vec4_repr():
    assert repr(Vec4F(0.1, 0.2, 0.3, 0.4)) == "LVector4f(0.1, 0.2, 0.3, 0.4)"
    assert repr(Vec4F(-0.9999999403953552, 1.000001, 1e-8, 0.0)) == "LVector4f(-0.99999994, 1.000001, 1e-8, 0)"
    assert repr(Vec4D(0.1, 0.2, 0.3, 0.4)) == "LVector4d(0.1, 0.2, 0.3, 0.4)"
    assert repr(Vec4D(-0.9999999403953552, 1.00000001, 1e-8, 0.0)) == "LVector4d(-0.9999999403953552, 1.00000001, 1e-8, 0)"


def test_vec4_buffer():
    v = Vec4(0, 0.5, 2.0, -4.0)
    m = memoryview(v)
    assert len(m) == 4
    assert m[0] == 0
    assert m[1] == 0.5
    assert m[2] == 2.0
    assert m[3] == -4.0
