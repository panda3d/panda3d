import pytest
from copy import copy
from panda3d import core


def test_mat3_aliases():
    assert core.LMatrix3 is core.Mat3
    assert core.LMatrix3f is core.Mat3F
    assert core.LMatrix3d is core.Mat3D

    assert (core.LMatrix3f is core.Mat3) != (core.LMatrix3d is core.Mat3)


@pytest.mark.parametrize("type", (core.LMatrix3f, core.LMatrix3d))
def test_mat3_constructor(type):
    # Test that three ways of construction produce the same matrix.
    mat1 = type((1, 2, 3),
                (4, 5, 6),
                (7, 8, 9))

    mat2 = type(1, 2, 3, 4, 5, 6, 7, 8, 9)

    mat3 = type((1, 2, 3, 4, 5, 6, 7, 8, 9))

    assert mat1 == mat2
    assert mat2 == mat3
    assert mat1 == mat3


@pytest.mark.parametrize("type", (core.LMatrix3d, core.LMatrix3f))
def test_mat3_copy_constuctor(type):
    mat1 = type((1, 2, 3),
                (4, 5, 6),
                (7, 8, 9))

    # Make a copy.  Changing it should not change the original.
    mat2 = type(mat1)
    assert mat1 == mat2
    mat2[0][0] = 100
    assert mat1 != mat2

    # Make a copy by unpacking.
    mat2 = type(*mat1)
    assert mat1 == mat2
    mat2[0][0] = 100
    assert mat1 != mat2

    # Make a copy by calling copy.copy.
    mat2 = copy(mat1)
    assert mat1 == mat2
    mat2[0][0] = 100
    assert mat1 != mat2


@pytest.mark.parametrize("type", (core.LMatrix3d, core.LMatrix3f))
def test_mat3_invert_same_type(type):
    mat = type((1, 0, 0,
                0, 1, 0,
                1, 2, 3))

    inv = core.invert(mat)
    assert mat.__class__ == inv.__class__
