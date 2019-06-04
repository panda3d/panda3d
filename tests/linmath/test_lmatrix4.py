import pytest
from copy import copy
from panda3d import core


def test_mat4_aliases():
    assert core.LMatrix4 is core.Mat4
    assert core.LMatrix4f is core.Mat4F
    assert core.LMatrix4d is core.Mat4D

    assert (core.LMatrix4f is core.Mat4) != (core.LMatrix4d is core.Mat4)


@pytest.mark.parametrize("type", (core.LMatrix4d, core.LMatrix4f))
def test_mat4_constructor(type):
    # Test that three ways of construction produce the same matrix.
    mat1 = type((1, 2, 3, 4),
                (5, 6, 7, 8),
                (9, 10, 11, 12),
                (13, 14, 15, 16))

    mat2 = type(1, 2, 3, 4,
                5, 6, 7, 8,
                9, 10, 11, 12,
                13, 14, 15, 16)

    mat3 = type((1, 2, 3, 4,
                 5, 6, 7, 8,
                 9, 10, 11, 12,
                 13, 14, 15, 16))

    assert mat1 == mat2
    assert mat2 == mat3
    assert mat1 == mat3


@pytest.mark.parametrize("type", (core.LMatrix4d, core.LMatrix4f))
def test_mat4_copy_constuctor(type):
    mat1 = type((1, 2, 3, 4),
                (5, 6, 7, 8),
                (9, 10, 11, 12),
                (13, 14, 15, 16))

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


@pytest.mark.parametrize("type", (core.LMatrix4d, core.LMatrix4f))
def test_mat4_invert_same_type(type):
    mat = type((1, 0, 0, 0,
                0, 1, 0, 0,
                0, 0, 1, 0,
                1, 2, 3, 1))

    inv = core.invert(mat)
    assert mat.__class__ == inv.__class__


@pytest.mark.parametrize("type", (core.LMatrix4d, core.LMatrix4f))
def test_mat4_invert_correct(type):
    mat = type((1, 0, 0, 0,
                0, 1, 0, 0,
                0, 0, 1, 0,
                1, 2, 3, 1))
    inv = type()
    assert inv.invert_from(mat)

    assert inv == type(( 1,  0,  0, 0,
                         0,  1,  0, 0,
                         0,  0,  1, 0,
                        -1, -2, -3, 1))

    assert (mat * inv).is_identity()
    assert (inv * mat).is_identity()


@pytest.mark.parametrize("type", (core.LMatrix4d, core.LMatrix4f))
def test_mat4_rows(type):
    mat = type((1, 2, 3, 4,
                5, 6, 7, 8,
                9, 10, 11, 12,
                13, 14, 15, 16))

    assert mat.rows[0] == (1, 2, 3, 4)
    assert mat.rows[1] == (5, 6, 7, 8)
    assert mat.rows[2] == (9, 10, 11, 12)
    assert mat.rows[3] == (13, 14, 15, 16)

    assert mat.get_row3(0) == (1, 2, 3)
    assert mat.get_row3(1) == (5, 6, 7)
    assert mat.get_row3(2) == (9, 10, 11)
    assert mat.get_row3(3) == (13, 14, 15)


@pytest.mark.parametrize("type", (core.LMatrix4d, core.LMatrix4f))
def test_mat4_cols(type):
    mat = type((1, 5, 9, 13,
                2, 6, 10, 14,
                3, 7, 11, 15,
                4, 8, 12, 16))

    assert mat.cols[0] == (1, 2, 3, 4)
    assert mat.cols[1] == (5, 6, 7, 8)
    assert mat.cols[2] == (9, 10, 11, 12)
    assert mat.cols[3] == (13, 14, 15, 16)

    assert mat.get_col3(0) == (1, 2, 3)
    assert mat.get_col3(1) == (5, 6, 7)
    assert mat.get_col3(2) == (9, 10, 11)
    assert mat.get_col3(3) == (13, 14, 15)
