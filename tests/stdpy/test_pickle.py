from direct.stdpy.pickle import dumps, loads, PicklingError
import pytest


def test_reduce_persist():
    from panda3d.core import NodePath

    parent = NodePath("parent")
    child = parent.attach_new_node("child")

    parent2, child2 = loads(dumps([parent, child]))
    assert tuple(parent2.children) == (child2,)


def test_pickle_copy():
    from panda3d.core import PandaNode, NodePath

    # Make two Python wrappers pointing to the same node
    node1 = PandaNode("node")
    node2 = NodePath(node1).node()
    assert node1.this == node2.this
    assert id(node1) != id(node2)

    # Test that pickling and loading still results in the same node object.
    node1, node2 = loads(dumps([node1, node2]))
    assert node1 == node2


def test_pickle_error():
    class ErroneousPickleable(object):
        def __reduce__(self):
            return 12345

    with pytest.raises(PicklingError):
        dumps(ErroneousPickleable())
