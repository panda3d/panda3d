import pytest

@pytest.fixture
def world():
    bullet = pytest.importorskip("panda3d.bullet")
    world = bullet.BulletWorld()

    return world

def simulate_until(world, cond, dt=1.0/60, limit=600):
    for x in range(limit):
        world.do_physics(dt)
        if cond():
            return True

    return False

@pytest.fixture
def scene(world):
    """
    This test scene contains a "ramp" which slopes down at a 45-degree angle at
    locations where X<0, and is flat at X>0. A bowling ball is placed above the
    ramp at X=-5, Z=7. A stack of 2 very light boxes is placed at X=5, Z=0.
    The world is given gravity.

    When running the Bullet simulation, the ball should fall, travel down the
    ramp, and hit the lower box with enough force to displace it and cause the
    upper box to topple to the ground.
    """

    core = pytest.importorskip("panda3d.core")
    bullet = pytest.importorskip("panda3d.bullet")

    bodies = []

    ramp = bullet.BulletRigidBodyNode('ramp')
    ramp.add_shape(bullet.BulletPlaneShape(core.Vec4(0, 0, 1, 0)))
    ramp.add_shape(bullet.BulletPlaneShape(core.Vec4(1, 0, 1, 0).normalized()))
    bodies.append(ramp)

    ball = bullet.BulletRigidBodyNode('ball')
    ball.add_shape(bullet.BulletSphereShape(1))
    ball.set_mass(100)
    ball.set_transform(core.TransformState.make_pos(core.Point3(-5, 0, 7)))
    bodies.append(ball)

    lower_box = bullet.BulletRigidBodyNode('lower_box')
    lower_box.add_shape(bullet.BulletBoxShape(core.Vec3(2.5)))
    lower_box.set_mass(1)
    lower_box.set_transform(core.TransformState.make_pos(core.Point3(5, 0, 2.5)))
    bodies.append(lower_box)

    upper_box = bullet.BulletRigidBodyNode('upper_box')
    upper_box.add_shape(bullet.BulletBoxShape(core.Vec3(2.5)))
    upper_box.set_mass(1)
    upper_box.set_transform(core.TransformState.make_pos(core.Point3(5, 0, 7.5)))
    bodies.append(upper_box)

    world.set_gravity(core.Vec3(0, 0, -9.8))

    scene = core.NodePath('scene')
    for body in bodies:
        scene.attach_new_node(body)
        world.attach(body)

    yield scene

    scene.remove_node()

    for body in bodies:
        world.remove(body)
