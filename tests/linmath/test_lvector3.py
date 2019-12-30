from math import floor, ceil
import sys

import pytest

from panda3d.core import Vec3


original_vector = Vec3(2.3, -2.6, 3.5)

reason = '''Rounding in Python 2.7 expects to return a float value, since it returns a Vector it
does not work. When Python 2.7 gets deprecated, remove this check.'''

@pytest.mark.skipif(sys.version_info < (3, 5), reason=reason)
def test_round():
    rounded_vector = round(original_vector)
    assert rounded_vector.x == 2
    assert rounded_vector.y == -3
    assert rounded_vector.z == 4


@pytest.mark.skipif(sys.version_info < (3, 5), reason=reason)
def test_floor():
    rounded_vector = floor(original_vector)
    assert rounded_vector.x == 2
    assert rounded_vector.y == -3
    assert rounded_vector.z == 3


@pytest.mark.skipif(sys.version_info < (3, 5), reason=reason)
def test_ceil():
    rounded_vector = ceil(original_vector)
    assert rounded_vector.x == 3
    assert rounded_vector.y == -2
    assert rounded_vector.z == 4
