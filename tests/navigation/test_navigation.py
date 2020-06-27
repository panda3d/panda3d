from direct.showbase.ShowBase import ShowBase
from panda3d import navigation
from panda3d import core
from panda3d.core import NodePath

def test_input_geom():
	
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
    prim.closePrimitive()

    geom = core.Geom(data)
    geom.add_primitive(prim)

    prim.add_vertices(2,3,5)
    prim.closePrimitive()
    geom.add_primitive(prim)

    node = core.GeomNode('gnode')
    node.addGeom(geom)

    scene = NodePath(node)
    # Defining navmesh as object of NavMehsBuilder class.
    nav = navigation.NavMeshBuilder()

    # Extracting geoms from 'scene' and calculating vertices and triangles.
    nav.fromNodePath(scene)
    # Finding number of vertices.
    vcount = nav.get_vert_count()
    # Finding number of triangles.
    tcount = nav.get_tri_count()
    assert vcount == 6
    assert tcount == 4

def test_poly_mesh():
	
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
    prim.closePrimitive()

    geom = core.Geom(data)
    geom.add_primitive(prim)

    prim.add_vertices(2,3,5)
    prim.closePrimitive()
    geom.add_primitive(prim)

    node = core.GeomNode('gnode')
    node.addGeom(geom)

    scene = NodePath(node)
    # Defining navmesh as object of NavMehsBuilder class.
    nav = navigation.NavMeshBuilder()

    # Extracting geoms from 'scene' and calculating vertices and triangles.
    nav.fromNodePath(scene)
    
    # Building the navmesh
    navmesh = nav.build()
    
    nverts = nav.getPmeshVertCount()
    npolys = nav.getPmeshPolyCount()
    maxpolys = nav.getPmeshMaxPolyCount()
    assert nverts == 230
    assert npolys == 112
    assert maxpolys == 226