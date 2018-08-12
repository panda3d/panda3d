import pytest
from panda3d import core


@pytest.mark.parametrize("type", (core.Mat4, core.Mat4D))
def test_mat4_invert(type):
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
