import pytest
from copy import copy
from panda3d import core


original_vector = core.Vector4f(2.3, 2.6, 3.5, 1)

def test_round():
    rounded_vector = round(original_vector)
    assert rounded_vector.x == 2
    assert rounded_vector.y == 3
    assert rounded_vector.z == 4
    assert rounded_vector.w == 1


def test_floor():
    rounded_vector = floor(original_vector)
    assert rounded_vector.x == 2
    assert rounded_vector.y == 2
    assert rounded_vector.z == 3
    assert rounded_vector.w == 1


def test_ceil():
    rounded_vector2 = ceil(original_vector)
    assert rounded_vector.x == 3
    assert rounded_vector.y == 3
    assert rounded_vector.y == 4
    assert rounded_vector.w == 1
