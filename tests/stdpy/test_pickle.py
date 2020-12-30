from direct.stdpy.pickle import dumps, loads, PicklingError
import pytest


def test_reduce_persist():
    from panda3d.core import NodePath

    parent = NodePath("parent")
    child = parent.attach_new_node("child")

    parent2, child2 = loads(dumps([parent, child]))
    assert tuple(parent2.children) == (child2,)


def test_pickle_error():
    class ErroneousPickleable(object):
        def __reduce__(self):
            return 12345

    with pytest.raises(PicklingError):
        dumps(ErroneousPickleable())
