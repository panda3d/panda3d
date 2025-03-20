from panda3d.core import *
import sys


class CustomObject:
    pass


def test_collision_node_owner():
    owner = CustomObject()
    initial_rc = sys.getrefcount(owner)

    node = CollisionNode("node")
    assert node.owner is None

    node.owner = owner
    assert sys.getrefcount(owner) == initial_rc
    assert node.owner is owner

    node.owner = owner
    assert sys.getrefcount(owner) == initial_rc
    assert node.owner is owner

    node.owner = None
    assert sys.getrefcount(owner) == initial_rc
    assert node.owner is None

    del node
    assert sys.getrefcount(owner) == initial_rc

    # Assign owner and then delete node
    node = CollisionNode("node")
    assert sys.getrefcount(owner) == initial_rc
    node.owner = owner
    assert sys.getrefcount(owner) == initial_rc
    del node
    assert sys.getrefcount(owner) == initial_rc

    # Delete owner and see what happens to the node
    node = CollisionNode("node")
    assert sys.getrefcount(owner) == initial_rc
    node.owner = owner
    assert sys.getrefcount(owner) == initial_rc
    del owner
    assert node.owner is None

