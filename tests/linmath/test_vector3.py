import sys
from math import floor, ceil

from panda3d.core import Vec3


original_vector = Vec3(2.3, 2.6, 3.5)

def test_round():
    # Rounding in python 2.7 expects to return a float value, since it returns a Vector it does not
    # work. When python 2.7 gets desprected, remove this check.
    if sys.version_info >= (3, 5):
        rounded_vector = round(original_vector)
        assert rounded_vector.x == 2
        assert rounded_vector.y == 3
        assert rounded_vector.z == 4


def test_floor():
    if sys.version_info >= (3, 5):
        rounded_vector = floor(original_vector)
        assert rounded_vector.x == 2
        assert rounded_vector.y == 2
        assert rounded_vector.z == 3


def test_ceil():
    if sys.version_info >= (3, 5):
        rounded_vector = ceil(original_vector)
        assert rounded_vector.x == 3
        assert rounded_vector.y == 3
        assert rounded_vector.z == 4
