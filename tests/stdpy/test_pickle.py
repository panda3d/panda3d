from direct.stdpy.pickle import dumps, loads


def test_reduce_persist():
    from panda3d.core import NodePath

    parent = NodePath("parent")
    child = parent.attach_new_node("child")

    parent2, child2 = loads(dumps([parent, child]))
    assert tuple(parent2.children) == (child2,)
