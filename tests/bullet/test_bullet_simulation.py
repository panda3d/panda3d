import pytest
from .conftest import simulate_until

# Skip these tests if we can't import bullet.
bullet = pytest.importorskip("panda3d.bullet")
from panda3d import core


def test_basics(world, scene):
    # N.B. see `scene` fixture's docstring in conftest.py to understand what's
    # being simulated here

    # Step forward until the ball crosses the threshold
    ball = scene.find('**/ball')
    assert simulate_until(world, lambda: ball.get_x() >= 0)

    # Continue simulating until upper box falls
    upper_box = scene.find('**/upper_box')
    assert upper_box.get_z() > 5.0
    assert simulate_until(world, lambda: upper_box.get_z() < 5.0)

def test_restitution(world, scene):
    ball = scene.find('**/ball')
    scene.find('**/ramp').node().restitution = 1.0

    for with_bounce in (False, True):
        # Reset ball
        ball.node().set_angular_velocity(core.Vec3(0))
        ball.node().set_linear_velocity(core.Vec3(0))
        ball.set_pos(-2, 0, 100)

        ball.node().restitution = 1.0 * with_bounce

        # Simulate until ball rolls/bounces across Y axis
        assert simulate_until(world, lambda: ball.get_x() >= 0)

        if with_bounce:
            # The ball bounced across, so it should be off the ground a bit
            assert ball.get_z() > 1.2
        else:
            # The ball rolled, so it should be on the ground
            assert ball.get_z() < 1.2

def test_friction(world, scene):
    ball = scene.find('**/ball')

    for with_friction in (False, True):
        # Reset ball, give it a huge negative (CCW) spin about the X axis so
        # it'll roll in +Y direction if there's any friction
        ball.node().set_angular_velocity(core.Vec3(-1000,0,0))
        ball.node().set_linear_velocity(core.Vec3(0))
        ball.set_pos(-2, 0, 5)

        ball.node().friction = 1.0 * with_friction

        # Simulate until ball crosses Y axis
        assert simulate_until(world, lambda: ball.get_x() >= 0)

        if with_friction:
            # The ball had friction, so should've gone off in the +Y direction
            assert ball.get_y() > 1
        else:
            # No friction means the Y axis should be unaffected
            assert abs(ball.get_y()) < 0.1
