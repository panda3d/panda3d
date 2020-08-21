import pytest


def test_odejoint_attach_both(world):
    from panda3d import ode

    body1 = ode.OdeBody(world)
    body2 = ode.OdeBody(world)

    assert len(body1.joints) == 0
    assert len(body2.joints) == 0

    joint = ode.OdeBallJoint(world)
    joint.attach(body1, body2)

    assert tuple(body1.joints) == (joint,)
    assert tuple(body2.joints) == (joint,)


def test_odejoint_attach_0(world):
    from panda3d import ode

    body = ode.OdeBody(world)

    assert len(body.joints) == 0

    joint = ode.OdeBallJoint(world)
    joint.attach(body, None)

    assert tuple(body.joints) == (joint,)


def test_odejoint_attach_1(world):
    from panda3d import ode

    body = ode.OdeBody(world)

    assert len(body.joints) == 0

    joint = ode.OdeBallJoint(world)
    joint.attach(None, body)

    assert tuple(body.joints) == (joint,)
