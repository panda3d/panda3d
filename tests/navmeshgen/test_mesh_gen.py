import pytest
from panda3d import core


def test_input_geom():
    navigation = pytest.importorskip("panda3d.navigation")
    navmeshgen = pytest.importorskip("panda3d.navmeshgen")
    data = core.GeomVertexData("", core.GeomVertexFormat.get_v3(), core.Geom.UH_static)
    vertex = core.GeomVertexWriter(data, "vertex")
    vertex.add_data3((0, 0, 0))
    vertex.add_data3((100, 1000, 0))
    vertex.add_data3((0, 100, 2))
    vertex.add_data3((500, 200, 2))
    vertex.add_data3((170, 20, 2))
    vertex.add_data3((100, 100, 100))

    prim = core.GeomTriangles(core.Geom.UH_static)
    prim.add_vertices(0, 1, 4)
    prim.close_primitive()

    geom = core.Geom(data)
    geom.add_primitive(prim)

    prim.add_vertices(2,3,5)
    prim.close_primitive()
    geom.add_primitive(prim)

    node = core.GeomNode('gnode')
    node.addGeom(geom)

    scene = core.NodePath(node)
    
    builder = navmeshgen.NavMeshBuilder()

    # Extracting geoms from 'scene' and calculating vertices and triangles.
    builder.from_node_path(scene)
    # Finding number of vertices.
    vcount = builder.get_vert_count()
    # Finding number of triangles.
    tcount = builder.get_tri_count()
    assert vcount == 6
    assert tcount == 4


def test_poly_mesh():
    navigation = pytest.importorskip("panda3d.navigation")
    navmeshgen = pytest.importorskip("panda3d.navmeshgen")
    data = core.GeomVertexData("", core.GeomVertexFormat.get_v3(), core.Geom.UH_static)
    vertex = core.GeomVertexWriter(data, "vertex")
    vertex.add_data3((0, 0, 0))
    vertex.add_data3((100, 1000, 0))
    vertex.add_data3((0, 100, 2))
    vertex.add_data3((500, 200, 2))
    vertex.add_data3((170, 20, 2))
    vertex.add_data3((100, 100, 100))

    prim = core.GeomTriangles(core.Geom.UH_static)
    prim.add_vertices(0, 1, 2)
    prim.close_primitive()

    geom = core.Geom(data)
    geom.add_primitive(prim)

    prim.add_vertices(2,3,5)
    prim.close_primitive()
    geom.add_primitive(prim)

    node = core.GeomNode('gnode')
    node.add_geom(geom)

    scene = core.NodePath(node)
    
    builder = navmeshgen.NavMeshBuilder()

    # Extracting geoms from 'scene' and calculating vertices and triangles.
    builder.from_node_path(scene)
    
    # Building the navmesh
    builder.build()
    
    nverts = builder.get_pmesh_vert_count()
    npolys = builder.get_pmesh_poly_count()
    assert nverts == 230
    assert npolys == 112
    assert builder.vert_count == 6
    assert builder.tri_count == 4
