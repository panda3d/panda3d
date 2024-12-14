import pytest, sys

def test_nodepath_empty():
    """Tests NodePath behavior for empty NodePaths."""
    from panda3d.core import NodePath, ParamNodePath
    import pickle

    empty = NodePath()
    assert empty.is_empty()
    assert not empty

    # Try pickling, which uses __reduce__
    dumped = pickle.dumps(empty)
    empty2 = pickle.loads(dumped)
    assert empty2.is_empty()
    assert not empty2
    assert empty == empty2

    # Test write_datagram/fillin, which are invoked when the NodePath is being
    # serialized indirectly, such as via ParamNodePath
    dumped = pickle.dumps(ParamNodePath(empty))
    empty2 = pickle.loads(dumped).get_value()
    assert empty2.is_empty()
    assert not empty2
    assert empty == empty2

def test_nodepath_single():
    """Tests NodePath behavior for single-node NodePaths."""
    from panda3d.core import NodePath

    np = NodePath('np')
    assert not np.is_empty()
    assert np

    assert np.get_pos() == (0, 0, 0)
    assert np.get_hpr() == (0, 0, 0)
    assert np.get_scale() == (1, 1, 1)

def test_nodepath_parent():
    """Tests NodePath.reparentTo()."""
    from panda3d.core import NodePath

    np1 = NodePath('np')
    np2 = NodePath('np')

    assert np1.parent is None
    assert np2.parent is None

    np1.reparentTo(np2)

    assert np1.parent == np2
    assert np2.parent is None

def test_nodepath_transform_changes():
    """Tests that NodePath applies transform changes to its managed node."""
    from panda3d.core import NodePath

    np = NodePath('np')
    assert np.get_pos() == (0, 0, 0)
    assert np.get_hpr() == (0, 0, 0)
    assert np.get_scale() == (1, 1, 1)

    np.set_pos(1, 2, 3)
    assert np.get_pos() == (1, 2, 3)
    assert np.node().get_transform().get_pos() == (1, 2, 3)

def test_nodepath_transform_composition():
    """Tests that NodePath composes transform states according to the path it holds."""
    from panda3d.core import PandaNode, NodePath, LPoint3, LVector3

    # Create 3 PandaNodes, and give each some interesting transform state:
    node1 = PandaNode('node1')
    node2 = PandaNode('node2')
    node3 = PandaNode('node3')

    node1.set_transform(node1.get_transform().set_pos(LPoint3(0, 0, 1)).set_hpr(LVector3(90, 0, -90)))
    node2.set_transform(node2.get_transform().set_pos(LPoint3(0, 1, 0)).set_hpr(LVector3(180, 180, 0)))
    node3.set_transform(node3.get_transform().set_pos(LPoint3(1, 0, 0)).set_hpr(LVector3(270, 0, 270)))

    # node3 is going to be attached under both node1 and node2 and we will
    # hold a path both ways:
    node1.add_child(node3)
    node2.add_child(node3)

    assert len(node1.children) == 1
    assert len(node2.children) == 1
    assert len(node3.children) == 0
    assert len(node1.parents) == 0
    assert len(node2.parents) == 0
    assert len(node3.parents) == 2

    # np1 is the path to node3 via node1:
    np1 = NodePath(node1).children[0]
    # np2 is the path to node3 via node2:
    np2 = NodePath(node2).children[0]

    # Both should point to node3:
    assert np1.node() == node3
    assert np2.node() == node3

    # However if we ask for the net transform to node3, it should compose:
    assert np1.get_transform(NodePath()) == node1.get_transform().compose(node3.get_transform())
    assert np2.get_transform(NodePath()) == node2.get_transform().compose(node3.get_transform())

    # If we ask for np1 RELATIVE to np2, it should compose like so:
    leg1 = node2.get_transform().compose(node3.get_transform())
    leg2 = node1.get_transform().compose(node3.get_transform())
    relative_transform = leg1.get_inverse().compose(leg2)
    assert np1.get_transform(np2).compare_to(relative_transform, True) == 0


def test_nodepath_comparison():
    from panda3d.core import NodePath, PandaNode

    path = NodePath("node")

    # Empty NodePath equals itself
    assert NodePath() == NodePath()
    assert not (NodePath() != NodePath())
    assert not (NodePath() > NodePath())
    assert not (NodePath() < NodePath())
    assert NodePath().compare_to(NodePath()) == 0

    # Empty NodePath does not equal non-empty NodePath
    assert NodePath() != path
    assert not (NodePath() == path)
    assert NodePath().compare_to(path) != 0
    assert path != NodePath()
    assert not (path == NodePath())
    assert path.compare_to(NodePath()) != 0

    # Copy of NodePath equals original
    path2 = NodePath(path)
    assert path == path2
    assert path2 == path
    assert not (path != path2)
    assert not (path2 != path)
    assert not (path > path2)
    assert not (path < path2)
    assert path.compare_to(path2) == 0
    assert path2.compare_to(path) == 0

    # NodePath pointing to copy of node is not the same
    path2 = NodePath(path.node().make_copy())
    assert path != path2
    assert path2 != path
    assert not (path == path2)
    assert not (path2 == path)
    assert (path2 > path) or (path > path2)
    assert (path2 < path) or (path < path2)
    assert path.compare_to(path2) != 0
    assert path2.compare_to(path) != 0


def test_weak_nodepath_comparison():
    from panda3d.core import NodePath, WeakNodePath

    path = NodePath("node")
    weak = WeakNodePath(path)

    assert path == weak
    assert weak == path
    assert weak <= path
    assert path <= weak
    assert weak >= path
    assert path >= weak
    assert not (path != weak)
    assert not (weak != path)
    assert not (weak > path)
    assert not (path > weak)
    assert not (weak < path)
    assert not (path < weak)

    assert hash(path) == hash(weak)
    assert weak.get_node_path() == path
    assert weak.node() == path.node()


def test_nodepath_flatten_tags_identical():
    from panda3d.core import NodePath, PandaNode

    # Do flatten nodes with same tags
    node1 = PandaNode("node1")
    node1.set_tag("key", "value")
    node2 = PandaNode("node2")
    node2.set_tag("key", "value")

    path = NodePath("parent")
    path.node().add_child(node1)
    path.node().add_child(node2)

    path.flatten_strong()
    assert len(path.children) == 1


def test_nodepath_flatten_tags_same_key():
    from panda3d.core import NodePath, PandaNode

    # Don't flatten nodes with different tag keys
    node1 = PandaNode("node1")
    node1.set_tag("key1", "value")
    node2 = PandaNode("node2")
    node2.set_tag("key2", "value")

    path = NodePath("parent")
    path.node().add_child(node1)
    path.node().add_child(node2)

    path.flatten_strong()
    assert len(path.children) == 2


def test_nodepath_flatten_tags_same_value():
    from panda3d.core import NodePath, PandaNode

    # Don't flatten nodes with different tag values
    node1 = PandaNode("node1")
    node1.set_tag("key", "value1")
    node2 = PandaNode("node2")
    node2.set_tag("key", "value2")

    path = NodePath("parent")
    path.node().add_child(node1)
    path.node().add_child(node2)

    path.flatten_strong()
    assert len(path.children) == 2


def test_nodepath_python_tags():
    from panda3d.core import NodePath

    path = NodePath("node")

    with pytest.raises(KeyError):
        path.python_tags["foo"]

    path.python_tags["foo"] = "bar"

    assert path.python_tags["foo"] == "bar"

    # Make sure reference count stays the same
    rc1 = sys.getrefcount(path.python_tags)
    rc2 = sys.getrefcount(path.python_tags)
    assert rc1 == rc2


def test_nodepath_clear_python_tag():
    from panda3d.core import NodePath

    path = NodePath("node")
    assert not path.has_python_tag("a")
    assert not path.has_python_tag("b")
    assert not path.node().has_tags()

    path.set_python_tag("a", "value")
    assert path.has_python_tag("a")
    assert not path.has_python_tag("b")
    assert path.node().has_tags()

    path.set_python_tag("b", "value")
    assert path.has_python_tag("a")
    assert path.has_python_tag("b")
    assert path.node().has_tags()

    path.clear_python_tag("a")
    assert not path.has_python_tag("a")
    assert path.has_python_tag("b")
    assert path.node().has_tags()

    path.clear_python_tag("b")
    assert not path.has_python_tag("a")
    assert not path.has_python_tag("b")
    assert not path.node().has_tags()


def test_nodepath_replace_texture():
    from panda3d.core import NodePath, Texture

    tex1 = Texture()
    tex2 = Texture()

    path1 = NodePath("node1")
    path1.set_texture(tex1)
    path1.replace_texture(tex1, tex2)
    assert path1.get_texture() == tex2

    path1 = NodePath("node1")
    path2 = path1.attach_new_node("node2")
    path2.set_texture(tex1)
    path1.replace_texture(tex1, tex2)
    assert not path1.has_texture()
    assert path2.get_texture() == tex2


def test_nodepath_replace_texture_none():
    from panda3d.core import NodePath, Texture

    tex1 = Texture("tex1")

    path1 = NodePath("node1")
    assert path1.get_texture() is None
    path1.set_texture(tex1)
    assert path1.get_texture() == tex1
    path1.replace_texture(tex1, None)
    assert path1.get_texture() is None

    path1 = NodePath("node1")
    path2 = path1.attach_new_node("node2")
    assert path2.get_texture() is None
    path2.set_texture(tex1)
    assert path2.get_texture() == tex1
    path1.replace_texture(tex1, None)
    assert path2.get_texture() is None


def test_nodepath_set_collide_owner():
    from panda3d.core import NodePath, CollisionNode

    class CustomOwner:
        pass

    owner1 = CustomOwner()
    owner2 = CustomOwner()
    owner3 = CustomOwner()

    root = NodePath("root")
    model1 = root.attach_new_node("model1")
    collider1 = model1.attach_new_node(CollisionNode("collider1"))
    collider2 = model1.attach_new_node(CollisionNode("collider2"))
    model2 = root.attach_new_node("model2")
    collider3 = model2.attach_new_node(CollisionNode("collider3"))

    root.set_collide_owner(owner1)
    assert collider1.node().owner is owner1
    assert collider2.node().owner is owner1
    assert collider3.node().owner is owner1

    model1.set_collide_owner(None)
    assert collider1.node().owner is None
    assert collider2.node().owner is None
    assert collider3.node().owner is owner1

    collider2.set_collide_owner(owner2)
    assert collider1.node().owner is None
    assert collider2.node().owner is owner2
    assert collider3.node().owner is owner1

    del owner1
    assert collider1.node().owner is None
    assert collider2.node().owner is owner2
    assert collider3.node().owner is None

    root.set_collide_owner(owner3)
    model2.set_collide_owner(owner2)
    assert collider1.node().owner is owner3
    assert collider2.node().owner is owner3
    assert collider3.node().owner is owner2
