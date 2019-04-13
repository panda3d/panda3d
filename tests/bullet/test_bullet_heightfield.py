import pytest

# Skip these tests if we can't import bullet.
bullet = pytest.importorskip("panda3d.bullet")

from panda3d.bullet import BulletWorld, BulletRigidBodyNode, ZUp
from panda3d.bullet import BulletHeightfieldShape, BulletSphereShape
from panda3d.core import NodePath, PNMImage

def make_node(name, BulletShape, *args):
    shape = BulletShape(*args)
    node = BulletRigidBodyNode(name)
    node.add_shape(shape)
    return node

def test_sphere_into_heightfield():
    root = NodePath("root")
    world = BulletWorld()

    sphere = make_node("Sphere", BulletSphereShape, 1)

    img = PNMImage(10, 10, 1)
    img.fill_val(255)
    heightfield = make_node("Heightfield", BulletHeightfieldShape, img, 1, ZUp)
    
    np1 = root.attach_new_node(sphere)
    np1.set_pos(0, 0, 1)
    world.attach(sphere)

    np2 = root.attach_new_node(heightfield)
    np2.set_pos(0, 0, 0)
    world.attach(heightfield)

    assert world.get_num_rigid_bodies() == 2
    test = world.contact_test_pair(sphere, heightfield)
    assert test.get_num_contacts() > 0
    assert test.get_contact(0).get_node0() == sphere
    assert test.get_contact(0).get_node1() == heightfield

    # No longer colliding
    np1.set_pos(0, 0, 2)
    test = world.contact_test_pair(sphere, heightfield)
    assert test.get_num_contacts() == 0
