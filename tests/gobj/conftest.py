import pytest
from panda3d.core import GeomVertexArrayFormat, GeomVertexFormat, GeomVertexData, GeomVertexWriter, Geom, GeomNode

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
