from panda3d.core import GeomVertexArrayFormat, GeomVertexFormat, GeomVertexData, Geom, GeomNode
from panda3d.core import GeomVertexReader, GeomVertexWriter
import pytest


@pytest.fixture(scope='module')
def geom_node():
    array = GeomVertexArrayFormat()
    array.add_column("vertex", 3, Geom.NT_float32, Geom.C_point)
    array.add_column("normal", 3, Geom.NT_float32, Geom.C_normal)
    array.add_column("color", 3, Geom.NT_float32, Geom.C_color)
    array.add_column("texcoord", 3, Geom.NT_float32, Geom.C_texcoord)
    format = GeomVertexFormat()
    format.add_array(array)
    format = GeomVertexFormat.register_format(format)

    vdata = GeomVertexData("test", format, Geom.UH_static)
    vdata.set_num_rows(4)
    vertex = GeomVertexWriter(vdata, 'vertex')
    normal = GeomVertexWriter(vdata, 'normal')
    color = GeomVertexWriter(vdata, 'color')
    texcoord = GeomVertexWriter(vdata, 'texcoord')

    vertex.add_data3f(1, 0, 0)
    normal.add_data3f(0, 0, 1)
    color.add_data4f(0, 0, 1, 1)
    texcoord.add_data2f(1, 0)

    vertex.add_data3f(1, 1, 0)
    normal.add_data3f(0, 1, 1)
    color.add_data4f(1, 0, 1, 1)
    texcoord.add_data2f(1, 1)

    geom = Geom(vdata)
    node = GeomNode('gnode')
    node.add_geom(geom)
    return node


def test_geom_vertex_reader(geom_node):
    new_geom = geom_node.get_geom(0)
    new_vdata = new_geom.get_vertex_data()

    new_vertex = GeomVertexReader(new_vdata, 'vertex')
    new_normal = GeomVertexReader(new_vdata, 'normal')
    new_color = GeomVertexReader(new_vdata, 'color')
    new_texcoord = GeomVertexReader(new_vdata, 'texcoord')

    assert new_vertex.get_data3f() == (1, 0, 0)
    assert new_vertex.get_data3f() == (1, 1, 0)

    assert new_normal.get_data3f() == (0, 0, 1)
    assert new_normal.get_data3f() == (0, 1, 1)

    assert new_color.get_data4f() == (0, 0, 1, 1)
    assert new_color.get_data4f() == (1, 0, 1, 1)

    assert new_texcoord.get_data2f() == (1, 0)
    assert new_texcoord.get_data2f() == (1, 1)
