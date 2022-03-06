from panda3d.core import PandaNode, TransformState


def test_node_prev_transform():
    identity = TransformState.make_identity()
    t1 = TransformState.make_pos((1, 0, 0))
    t2 = TransformState.make_pos((2, 0, 0))
    t3 = TransformState.make_pos((3, 0, 0))

    node = PandaNode("node")
    assert node.transform == identity
    assert node.prev_transform == identity
    assert not node.has_dirty_prev_transform()

    node.transform = t1
    assert node.transform == t1
    assert node.prev_transform == identity
    assert node.has_dirty_prev_transform()

    node.transform = t2
    assert node.transform == t2
    assert node.prev_transform == identity
    assert node.has_dirty_prev_transform()

    node.reset_prev_transform()
    assert node.transform == t2
    assert node.prev_transform == t2
    assert not node.has_dirty_prev_transform()

    node.transform = t3
    assert node.prev_transform == t2
    assert node.has_dirty_prev_transform()
    PandaNode.reset_all_prev_transform()
    assert node.transform == t3
    assert node.prev_transform == t3
    assert not node.has_dirty_prev_transform()
