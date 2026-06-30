import pytest

from panda3d import core

def test_floor():
    root = core.NodePath('root')
    trav = core.CollisionTraverser()

    character = root.attach_new_node(core.PandaNode('char'))
    sphere = core.CollisionNode('sphere')
    sphere.add_solid(core.CollisionSphere(center=(0, 0, 0), radius=1))
    sphere = character.attach_new_node(sphere)

    floor = core.CollisionNode('floor')
    floor.add_solid(core.CollisionPlane(core.LPlane()))
    floor = root.attach_new_node(floor)
    floor.set_z(0.25)

    handler = core.CollisionHandlerFloor()
    handler.add_collider(sphere, character)
    trav.add_collider(sphere, handler)

    assert character.get_z() == pytest.approx(0)
    trav.traverse(root)
    assert character.get_z() == pytest.approx(0.25)
    character.set_z(-0.25)
    trav.traverse(root)
    assert character.get_z() == pytest.approx(0.25)
