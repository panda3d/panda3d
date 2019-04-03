import pytest
from panda3d.core import GeomVertexReader

def test_geom_vertex_reader(geom_node):
    new_geom = geom_node.get_geom(0)
    new_vdata = new_geom.get_vertex_data()

    new_vertex = GeomVertexReader(new_vdata, 'vertex')
    new_normal = GeomVertexReader(new_vdata, 'normal')
    new_color = GeomVertexReader(new_vdata, 'color')
    new_texcoord = GeomVertexReader(new_vdata, 'texcoord')

    assert new_vertex.get_data3f()==(1,0,0)
    assert new_vertex.get_data3f() == (1, 1, 0)

    assert new_normal.get_data3f() == (0, 0, 1)
    assert new_normal.get_data3f() == (0, 1, 1)

    assert new_color.get_data4f() == (0, 0, 1, 1)
    assert new_color.get_data4f() == (1, 0, 1, 1)

    assert new_texcoord.get_data2f() == (1, 0)
    assert new_texcoord.get_data2f() == (1, 1)
