import pytest
physics = pytest.importorskip("panda3d.physics")

from direct.particles.GlobalForceGroup import GlobalForceGroup


def test_GlobalForceGroup():
    gfg = GlobalForceGroup()

    force1 = physics.LinearVectorForce((1, 0, 0))
    force2 = physics.LinearVectorForce((0, 1, 0))
    gfg.addForce(force1)
    assert tuple(gfg) == (force1,)
    gfg.addForce(force2)
    assert tuple(gfg) == (force1, force2)
    gfg.removeForce(force1)
    assert tuple(gfg) == (force2,)
    gfg.removeForce(force1)
    assert tuple(gfg) == (force2,)
    gfg.removeForce(force2)
    assert tuple(gfg) == ()
