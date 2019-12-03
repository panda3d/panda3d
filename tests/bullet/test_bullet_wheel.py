import pytest
from pytest import approx
# Skip these tests if we can't import bullet.
bullet = pytest.importorskip("panda3d.bullet")

from panda3d.core import Vec3
from panda3d.bullet import BulletWorld
from panda3d.bullet import BulletBoxShape
from panda3d.bullet import BulletRigidBodyNode
from panda3d.bullet import BulletVehicle


def test_get_steering():
    world = BulletWorld()
    # Chassis
    shape = BulletBoxShape(Vec3(0.6, 1.4, 0.5))
    body = BulletRigidBodyNode('Vehicle')
    body.addShape(shape)
    world.attach(body)
    # Vehicle
    vehicle = BulletVehicle(world, body)
    world.attachVehicle(vehicle)
    # Wheel
    wheel = vehicle.createWheel()
    wheel.setSteering(30.0)
    # Returns the steering angle in degrees.
    assert wheel.getSteering() == approx(30.0)
