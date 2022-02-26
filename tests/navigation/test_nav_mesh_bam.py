from panda3d import core


def reconstruct(object):
    # Create a temporary buffer, which we first write the object into, and
    # subsequently read it from again.
    
    buffer = core.DatagramBuffer()

    writer = core.BamWriter(buffer)
    writer.init()
    writer.write_object(object)

    reader = core.BamReader(buffer)
    reader.init()
    object = reader.read_object()
    reader.resolve()
    return object


def test_nav_mesh_bam(navmeshgen, navigation):
    data = core.GeomVertexData("", core.GeomVertexFormat.get_v3(), core.Geom.UH_static)
    vertex = core.GeomVertexWriter(data, "vertex")
    vertex.add_data3((0, 0, 0))
    vertex.add_data3((100, 0, 3))
    vertex.add_data3((100, 200, 1))
    vertex.add_data3((50, 50, 4))

    prim = core.GeomTriangles(core.Geom.UH_static)
    prim.add_vertices(0, 1, 2)
    prim.close_primitive()

    geom = core.Geom(data)
    geom.add_primitive(prim)

    prim.add_vertices(3, 1, 2)
    prim.close_primitive()

    node = core.GeomNode('gnode')
    node.add_geom(geom)

    scene = core.NodePath(node)

    # Defining navmesh as object of NavMehsBuilder class.
    builder = navmeshgen.NavMeshBuilder()

    # Extracting geoms from 'scene' and calculating vertices and triangles.
    builder.from_node_path(scene)

    navmesh = builder.build()

    vertCount = navmesh.get_vert_count()
    polyCount = navmesh.get_poly_count()
    detailVertsCount = navmesh.get_detail_vert_count()
    detailTriCount = navmesh.get_detail_tri_count()

    navmeshBam = reconstruct(navmesh)

    assert type(navmesh) == type(navmeshBam)
    assert vertCount == navmeshBam.get_vert_count()
    assert polyCount == navmeshBam.get_poly_count()
    assert detailVertsCount == navmeshBam.get_detail_vert_count()
    assert detailTriCount == navmeshBam.get_detail_tri_count()


def test_nav_mesh_node_bam(navmeshgen, navigation):
    data = core.GeomVertexData("", core.GeomVertexFormat.get_v3(), core.Geom.UH_static)
    vertex = core.GeomVertexWriter(data, "vertex")
    vertex.add_data3((0, 0, 0))
    vertex.add_data3((100, 0, 3))
    vertex.add_data3((100, 200, 1))
    vertex.add_data3((50, 50, 4))

    prim = core.GeomTriangles(core.Geom.UH_static)
    prim.add_vertices(0, 1, 2)
    prim.close_primitive()

    geom = core.Geom(data)
    geom.add_primitive(prim)

    prim.add_vertices(3, 1, 2)
    prim.close_primitive()

    node = core.GeomNode('gnode')
    node.add_geom(geom)

    scene = core.NodePath(node)

    # Defining navmesh as object of NavMehsBuilder class.
    builder = navmeshgen.NavMeshBuilder()

    # Extracting geoms from 'scene' and calculating vertices and triangles.
    builder.from_node_path(scene)

    navmesh = builder.build()

    navmeshnode = navigation.NavMeshNode("navmeshnode",navmesh)
    vertCount = navmeshnode.get_nav_mesh().get_vert_count()
    polyCount = navmeshnode.get_nav_mesh().get_poly_count()
    detailVertsCount = navmeshnode.get_nav_mesh().get_detail_vert_count()
    detailTriCount = navmeshnode.get_nav_mesh().get_detail_tri_count()

    navmeshNodeBam = reconstruct(navmeshnode)

    assert type(navmeshnode) == type(navmeshNodeBam)
    assert vertCount == navmeshNodeBam.get_nav_mesh().get_vert_count()
    assert polyCount == navmeshNodeBam.get_nav_mesh().get_poly_count()
    assert detailVertsCount == navmeshNodeBam.get_nav_mesh().get_detail_vert_count()
    assert detailTriCount == navmeshNodeBam.get_nav_mesh().get_detail_tri_count()


def test_nav_query_bam(navmeshgen, navigation):
    data = core.GeomVertexData("", core.GeomVertexFormat.get_v3(), core.Geom.UH_static)
    vertex = core.GeomVertexWriter(data, "vertex")
    vertex.add_data3((0, 0, 0))
    vertex.add_data3((100, 0, 3))
    vertex.add_data3((100, 200, 1))
    vertex.add_data3((50, 50, 4))

    prim = core.GeomTriangles(core.Geom.UH_static)
    prim.add_vertices(0, 1, 2)
    prim.close_primitive()

    geom = core.Geom(data)
    geom.add_primitive(prim)

    prim.add_vertices(3, 1, 2)
    prim.close_primitive()

    node = core.GeomNode('gnode')
    node.add_geom(geom)

    scene = core.NodePath(node)

    # Defining navmesh as object of NavMehsBuilder class.
    builder = navmeshgen.NavMeshBuilder()

    # Extracting geoms from 'scene' and calculating vertices and triangles.
    builder.from_node_path(scene)

    navmesh = builder.build()

    query = navigation.NavMeshQuery(navmesh)
    pos = core.LPoint3(50, 55, 3)
    query.nearest_point(pos)
    path = query.find_path(pos, core.LPoint3(100, 0, 3))

    navmeshBam = reconstruct(navmesh)
    queryBam = navigation.NavMeshQuery(navmeshBam)
    posBam = core.LPoint3(50, 55, 3)
    queryBam.nearest_point(posBam)
    pathBam = query.find_path(posBam, core.LPoint3(100, 0, 3))

    assert pos == posBam
    assert path == pathBam