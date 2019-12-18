from math import floor, ceil

from panda3d.core import Vec3


original_vector = Vec3(2.3, 2.6, 3.5)

def test_round():
    rounded_vector = round(original_vector)
    assert rounded_vector.x == 2
    assert rounded_vector.y == 3
    assert rounded_vector.z == 4


def test_floor():
    rounded_vector = floor(original_vector)
    assert rounded_vector.x == 2
    assert rounded_vector.y == 2
    assert rounded_vector.z == 3


def test_ceil():
    rounded_vector2 = ceil(original_vector)
    assert rounded_vector.x == 3
    assert rounded_vector.y == 3
    assert rounded_vector.y == 4
