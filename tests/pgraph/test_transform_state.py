from panda3d.core import TransformState, Mat4, Mat3


def test_transform_identity():
    state = TransformState.make_identity()

    assert state.is_identity()
    assert not state.is_invalid()
    assert not state.is_singular()
    assert state.is_2d()

    assert state.has_components()
    assert not state.components_given()
    assert not state.hpr_given()
    assert not state.quat_given()
    assert state.has_pos()
    assert state.has_hpr()
    assert state.has_quat()
    assert state.has_scale()
    assert state.has_identity_scale()
    assert state.has_uniform_scale()
    assert state.has_shear()
    assert not state.has_nonzero_shear()
    assert state.has_mat()

    assert state.get_pos() == (0, 0, 0)
    assert state.get_hpr() == (0, 0, 0)
    assert state.get_quat() == (1, 0, 0, 0)
    assert state.get_norm_quat() == (1, 0, 0, 0)
    assert state.get_scale() == (1, 1, 1)
    assert state.get_uniform_scale() == 1
    assert state.get_shear() == (0, 0, 0)
    assert state.get_mat() == Mat4.ident_mat()

    assert state.get_pos2d() == (0, 0)
    assert state.get_rotate2d() == 0
    assert state.get_scale2d() == (1, 1)
    assert state.get_shear2d() == 0
    assert state.get_mat3() == Mat3.ident_mat()

    state2 = TransformState.make_identity()
    assert state.this == state2.this


def test_transform_invalid():
    state = TransformState.make_invalid()

    assert not state.is_identity()
    assert state.is_invalid()
    assert state.is_singular()
    assert not state.is_2d()

    assert not state.has_components()
    assert not state.components_given()
    assert not state.hpr_given()
    assert not state.quat_given()
    assert not state.has_pos()
    assert not state.has_hpr()
    assert not state.has_quat()
    assert not state.has_scale()
    assert not state.has_identity_scale()
    assert not state.has_uniform_scale()
    assert not state.has_shear()
    assert not state.has_nonzero_shear()
    assert not state.has_mat()

    state2 = TransformState.make_invalid()
    assert state.this == state2.this
