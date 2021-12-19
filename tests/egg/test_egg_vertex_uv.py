import pytest
from panda3d import core

# Skip these tests if we can't import egg.
egg = pytest.importorskip("panda3d.egg")


def read_egg_vertex(string):
    """Reads an EggVertex from a string."""
    data = "<VertexPool> pool { <Vertex> 1 { %s } }" % (string)
    stream = core.StringStream(data.encode('utf-8'))
    data = egg.EggData()
    assert data.read(stream)
    pool, = data.get_children()
    return pool.get_vertex(1)


def test_egg_vertex_uv_empty():
    vertex = read_egg_vertex("""
        0 0 0
        <UV> {
            0 0
        }
    """)

    obj = vertex.get_uv_obj("")
    assert not obj.has_tangent()
    assert not obj.has_tangent4()

    assert '<Tangent>' not in str(obj)


def test_egg_vertex_tangent():
    vertex = read_egg_vertex("""
        0 0 0
        <UV> {
            0 0
            <Tangent> { 2 3 4 }
        }
    """)

    obj = vertex.get_uv_obj("")
    assert obj.has_tangent()
    assert not obj.has_tangent4()
    assert obj.get_tangent() == (2, 3, 4)
    assert obj.get_tangent4() == (2, 3, 4, 1)

    assert '{ 2 3 4 }' in str(obj)


def test_egg_vertex_tangent4_pos():
    vertex = read_egg_vertex("""
        0 0 0
        <UV> {
            0 0
            <Tangent> { 2 3 4 1 }
        }
    """)

    obj = vertex.get_uv_obj("")
    assert obj.has_tangent()
    assert obj.has_tangent4()
    assert obj.get_tangent() == (2, 3, 4)
    assert obj.get_tangent4() == (2, 3, 4, 1)

    assert '{ 2 3 4 1 }' in str(obj)


def test_egg_vertex_tangent4_neg():
    vertex = read_egg_vertex("""
        0 0 0
        <UV> {
            0 0
            <Tangent> { 2 3 4 -1 }
        }
    """)

    obj = vertex.get_uv_obj("")
    assert obj.has_tangent()
    assert obj.has_tangent4()
    assert obj.get_tangent() == (2, 3, 4)
    assert obj.get_tangent4() == (2, 3, 4, -1)

    assert '{ 2 3 4 -1 }' in str(obj)
