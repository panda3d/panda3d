import pytest
# Skip these tests if we can't import bullet.
bullet = pytest.importorskip("panda3d.bullet")

from panda3d.bullet import BulletWorld, BulletRigidBodyNode, BulletGhostNode
from panda3d.bullet import BulletBoxShape, BulletPlaneShape
from panda3d.core import NodePath, Vec3, TransformState


def test_singular_rigid_shape_bounds():
    root = NodePath("root")
    world = BulletWorld()

    # make test node
    box = BulletRigidBodyNode('box')
    boxNP = root.attach_new_node(box)
    #add shape
    box.addShape(BulletBoxShape(Vec3(0.5,0.5,0.5)), TransformState.makePos(Vec3(0.5,0,0)))
    world.attach(box)
    boxNP.setPos(0, 0, 0)

    #add plane to collide with
    plane = BulletRigidBodyNode('checkplane')
    planeNP = root.attach_new_node(plane)
    plane.addShape(BulletPlaneShape(Vec3(-1, 0, 0), 0))
    world.attach(plane)
    planeNP.set_pos(0.9, 0, 0)

    assert world.get_num_rigid_bodies() == 2
    test = world.contact_test_pair(box, plane)
    assert test.get_num_contacts() > 0
    assert test.get_contact(0).get_node0() == box
    assert test.get_contact(0).get_node1() == plane

    #move plane X coordinate, no longer colliding
    planeNP.set_pos(1.1, 0, 0)
    test = world.contact_test_pair(box, plane)
    assert test.get_num_contacts() == 0

def test_singular_rigid_double_scale_post_add():
    root = NodePath("root")
    world = BulletWorld()

    # make test node
    box = BulletRigidBodyNode('box')
    boxNP = root.attach_new_node(box)
    #add shape
    box.addShape(BulletBoxShape(Vec3(0.5,0.5,0.5)), TransformState.makePos(Vec3(0.5,0,0)))
    world.attach(box)
    boxNP.setPos(0, 0, 0)
    boxNP.setScale(2)

    #add plane to collide with
    plane = BulletRigidBodyNode('checkplane')
    planeNP = root.attach_new_node(plane)
    plane.addShape(BulletPlaneShape(Vec3(-1, 0, 0), 0))
    world.attach(plane)
    planeNP.set_pos(1.9, 0, 0)

    assert world.get_num_rigid_bodies() == 2
    test = world.contact_test_pair(box, plane)
    assert test.get_num_contacts() > 0
    assert test.get_contact(0).get_node0() == box
    assert test.get_contact(0).get_node1() == plane

    #move plane X coordinate, no longer colliding
    planeNP.set_pos(2.1, 0, 0)
    test = world.contact_test_pair(box, plane)
    assert test.get_num_contacts() == 0

def test_singular_rigid_double_initial_scale():
    root = NodePath("root")
    world = BulletWorld()

    # make test node
    box = BulletRigidBodyNode('box')
    boxNP = root.attach_new_node(box)
    boxNP.setScale(2)
    #add shape
    box.addShape(BulletBoxShape(Vec3(0.5,0.5,0.5)), TransformState.makePos(Vec3(0.5,0,0)))
    world.attach(box)
    boxNP.setPos(0, 0, 0)

    #add plane to collide with
    plane = BulletRigidBodyNode('checkplane')
    planeNP = root.attach_new_node(plane)
    plane.addShape(BulletPlaneShape(Vec3(-1, 0, 0), 0))
    world.attach(plane)
    planeNP.set_pos(1.9, 0, 0)

    assert world.get_num_rigid_bodies() == 2
    test = world.contact_test_pair(box, plane)
    assert test.get_num_contacts() > 0
    assert test.get_contact(0).get_node0() == box
    assert test.get_contact(0).get_node1() == plane

    #move plane X coordinate, no longer colliding
    planeNP.set_pos(2.1, 0, 0)
    test = world.contact_test_pair(box, plane)
    assert test.get_num_contacts() == 0

def test_composite_rigid_shape_bounds():
    root = NodePath("root")
    world = BulletWorld()

    # make test node
    box = BulletRigidBodyNode('boxes')
    boxNP = root.attach_new_node(box)
    #two boxes, next to each other
    box.addShape(BulletBoxShape(Vec3(0.5,0.5,0.5)), TransformState.makePos(Vec3(0.5,0,0)))
    box.addShape(BulletBoxShape(Vec3(0.5,0.5,0.5)), TransformState.makePos(Vec3(1.5,0,0)))
    world.attach(box)
    boxNP.setPos(0, 0, 0)

    #add plane to collide with
    plane = BulletRigidBodyNode('checkplane')
    planeNP = root.attach_new_node(plane)
    plane.addShape(BulletPlaneShape(Vec3(-1, 0, 0), 0))
    world.attach(plane)
    planeNP.set_pos(1.9, 0, 0)

    assert world.get_num_rigid_bodies() == 2
    test = world.contact_test_pair(box, plane)
    assert test.get_num_contacts() > 0
    assert test.get_contact(0).get_node0() == box
    assert test.get_contact(0).get_node1() == plane

    #move plane X coordinate, no longer colliding
    planeNP.set_pos(2.1, 0, 0)
    test = world.contact_test_pair(box, plane)
    assert test.get_num_contacts() == 0

def test_composite_rigid_double_scale_post_add():
    root = NodePath("root")
    world = BulletWorld()

    # make test node
    box = BulletRigidBodyNode('boxes')
    boxNP = root.attach_new_node(box)
    #two boxes, next to each other
    box.addShape(BulletBoxShape(Vec3(0.5,0.5,0.5)), TransformState.makePos(Vec3(0.5,0,0)))
    box.addShape(BulletBoxShape(Vec3(0.5,0.5,0.5)), TransformState.makePos(Vec3(1.5,0,0)))
    world.attach(box)
    boxNP.setPos(0, 0, 0)
    boxNP.setScale(2)

    #add plane to collide with
    plane = BulletRigidBodyNode('checkplane')
    planeNP = root.attach_new_node(plane)
    plane.addShape(BulletPlaneShape(Vec3(-1, 0, 0), 0))
    world.attach(plane)
    planeNP.set_pos(3.9, 0, 0)

    assert world.get_num_rigid_bodies() == 2
    test = world.contact_test_pair(box, plane)
    assert test.get_num_contacts() > 0
    assert test.get_contact(0).get_node0() == box
    assert test.get_contact(0).get_node1() == plane

    #move plane X coordinate, no longer colliding
    planeNP.set_pos(4.1, 0, 0)
    test = world.contact_test_pair(box, plane)
    assert test.get_num_contacts() == 0

def test_composite_rigid_double_initial_scale():
    root = NodePath("root")
    world = BulletWorld()

    # make test node
    box = BulletRigidBodyNode('boxes')
    boxNP = root.attach_new_node(box)
    boxNP.setScale(2)
    #two boxes, next to each other
    box.addShape(BulletBoxShape(Vec3(0.5,0.5,0.5)), TransformState.makePos(Vec3(0.5,0,0)))
    box.addShape(BulletBoxShape(Vec3(0.5,0.5,0.5)), TransformState.makePos(Vec3(1.5,0,0)))
    world.attach(box)
    boxNP.setPos(0, 0, 0)

    #add plane to collide with
    plane = BulletRigidBodyNode('checkplane')
    planeNP = root.attach_new_node(plane)
    plane.addShape(BulletPlaneShape(Vec3(-1, 0, 0), 0))
    world.attach(plane)
    planeNP.set_pos(3.9, 0, 0)

    assert world.get_num_rigid_bodies() == 2
    test = world.contact_test_pair(box, plane)
    assert test.get_num_contacts() > 0
    assert test.get_contact(0).get_node0() == box
    assert test.get_contact(0).get_node1() == plane

    #move plane X coordinate, no longer colliding
    planeNP.set_pos(4.1, 0, 0)
    test = world.contact_test_pair(box, plane)
    assert test.get_num_contacts() == 0
