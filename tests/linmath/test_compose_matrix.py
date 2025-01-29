from panda3d import core
import pytest


@pytest.mark.parametrize("coordsys", (core.CS_zup_right, core.CS_yup_right, core.CS_zup_left, core.CS_yup_left))
def test_compose_matrix(coordsys):
    scale = core.LVecBase3(1.2, 0.5, 2)
    hpr = core.LVecBase3(45, -90, 12.5)
    shear = core.LVecBase3(0, 0, 0)

    mat = core.LMatrix3()
    core.compose_matrix(mat, scale, shear, hpr, coordsys)

    new_scale = core.LVecBase3()
    new_hpr = core.LVecBase3()
    new_shear = core.LVecBase3()
    core.decompose_matrix(mat, new_scale, new_shear, new_hpr, coordsys)

    assert new_scale.almost_equal(scale)
    assert new_shear.almost_equal(shear)

    quat = core.LQuaternion()
    quat.set_hpr(hpr, coordsys)
    new_quat = core.LQuaternion()
    new_quat.set_hpr(new_hpr, coordsys)
    assert quat.is_same_direction(new_quat)


@pytest.mark.parametrize("coordsys", (core.CS_zup_right, core.CS_yup_right, core.CS_zup_left, core.CS_yup_left))
def test_compose_matrix2(coordsys):
    mat = core.LMatrix3(1, 0, 0, 0, 0, -1, 0, 1, 0)

    new_scale = core.LVecBase3()
    new_hpr = core.LVecBase3()
    new_shear = core.LVecBase3()
    core.decompose_matrix(mat, new_scale, new_shear, new_hpr, coordsys)

    assert new_scale.almost_equal(core.LVecBase3(1, 1, 1))
    if coordsys in (core.CS_zup_left, core.CS_yup_left):
        assert new_hpr.almost_equal(core.LVecBase3(0, 90, 0))
    else:
        assert new_hpr.almost_equal(core.LVecBase3(0, -90, 0))
    assert new_shear.almost_equal(core.LVecBase3(0, 0, 0))
