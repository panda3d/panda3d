from panda3d.core import PandaNode, TransformState
from contextlib import contextmanager
import pytest
import gc


@contextmanager
def gc_disabled():
    gc.disable()
    gc.collect()
    gc.freeze()
    gc.set_debug(gc.DEBUG_SAVEALL)
    gc.garbage.clear()

    try:
        yield
    finally:
        gc.set_debug(0)
        gc.garbage.clear()
        gc.unfreeze()
        gc.collect()
        gc.enable()


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


def test_node_tag_cycle():
    with gc_disabled():
        node = PandaNode('test_node_tag_cycle')
        node.set_python_tag('self', node)
        node.set_python_tag('self2', node)
        assert gc.is_tracked(node)
        node = None

        gc.collect()
        assert len(gc.garbage) > 0

        for g in gc.garbage:
            if isinstance(g, PandaNode) and g.name == 'test_node_tag_cycle':
                break
        else:
            assert False


@pytest.mark.xfail
def test_node_tag_cycle_multiple_wrappers():
    # Doesn't yet work since the traverse checks that the refcount is 1.
    with gc_disabled():
        node = PandaNode('test_node_tag_cycle_multiple_wrappers')
        node.set_python_tag('self', node)

        # Find another reference to the same node, we do this by temporarily
        # attaching a child.
        child = PandaNode('child')
        node.add_child(child)
        node2 = child.get_parent(0)
        node.remove_child(0)
        child = None

        assert node2 == node
        assert node2 is not node
        assert node2.this == node.this

        node.set_python_tag('self2', node2)

        node = None
        node2 = None

        gc.collect()
        assert len(gc.garbage) > 0

        for g in gc.garbage:
            if isinstance(g, PandaNode) and g.name == 'test_node_tag_cycle_multiple_wrappers':
                break
        else:
            pytest.fail('not found in garbage')


def test_node_subclass_persistent():
    class NodeSubclass(PandaNode):
        pass

    node = NodeSubclass('test_node_subclass')
    assert isinstance(node, PandaNode)

    # Always GC-tracked
    assert gc.is_tracked(node)

    # Registered type handle
    type_handle = node.get_type()
    assert type_handle.name == 'NodeSubclass'
    assert type_handle != PandaNode.get_class_type()
    assert tuple(type_handle.parent_classes) == (PandaNode.get_class_type(), )

    # Persistent wrapper
    parent = PandaNode('parent')
    parent.add_child(node)
    child = parent.get_child(0)
    assert child.this == node.this
    assert child is node
    assert type(child) is NodeSubclass
    parent = None
    child = None


def test_node_subclass_gc():
    class NodeSubclass(PandaNode):
        pass

    # Python wrapper destructs last
    with gc_disabled():
        node = NodeSubclass('test_node_subclass_gc1')

        assert node in gc.get_objects()

        node = None

        gc.collect()
        assert len(gc.garbage) == 1
        assert gc.garbage[0].name == 'test_node_subclass_gc1'

    # C++ reference destructs last
    with gc_disabled():
        node = NodeSubclass('test_node_subclass_gc2')
        parent = PandaNode('parent')
        parent.add_child(node)

        assert node in gc.get_objects()

        node = None

        # Hasn't been collected yet, since parent still holds a reference
        gc.collect()
        assert len(gc.garbage) == 0
        assert 'test_node_subclass_gc2' in [obj.name for obj in gc.get_objects() if isinstance(obj, PandaNode)]

        parent = None

        # Destructed without needing the GC
        assert 'test_node_subclass_gc2' not in [obj.name for obj in gc.get_objects() if isinstance(obj, PandaNode)]

        gc.collect()
        assert len(gc.garbage) == 0
