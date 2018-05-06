from panda3d import core


def test_geom_triangles_adjacency():
    prim = core.GeomTriangles(core.GeomEnums.UH_static)
    prim.add_vertex(0)
    prim.add_vertex(1)
    prim.add_vertex(2)
    prim.close_primitive()
    prim.add_vertex(2)
    prim.add_vertex(1)
    prim.add_vertex(3)
    prim.close_primitive()

    adj = prim.make_adjacency()
    verts = adj.get_vertex_list()
    assert tuple(verts) == (
        0, 0, 1, 3, 2, 2,
        2, 0, 1, 1, 3, 3,
    )


def test_geom_lines_adjacency():
    prim = core.GeomLines(core.GeomEnums.UH_static)
    prim.add_vertex(0)
    prim.add_vertex(1)
    prim.close_primitive()
    prim.add_vertex(1)
    prim.add_vertex(2)
    prim.close_primitive()
    prim.add_vertex(2)
    prim.add_vertex(3)
    prim.close_primitive()

    adj = prim.make_adjacency()
    verts = adj.get_vertex_list()
    assert tuple(verts) == (
        0, 0, 1, 2,
        0, 1, 2, 3,
        1, 2, 3, 3,
    )


def test_geom_linestrips_adjacency():
    prim = core.GeomLinestrips(core.GeomEnums.UH_static)
    prim.add_vertex(0)
    prim.add_vertex(1)
    prim.close_primitive()
    prim.add_vertex(1)
    prim.add_vertex(2)
    prim.add_vertex(3)
    prim.close_primitive()
    prim.add_vertex(3)
    prim.add_vertex(4)
    prim.add_vertex(5)
    prim.add_vertex(6)
    prim.close_primitive()

    adj = prim.make_adjacency()
    verts = adj.get_vertex_list()
    cut = adj.get_strip_cut_index()
    assert tuple(verts) == (
        0, 0, 1, 2,
        cut,
        0, 1, 2, 3, 4,
        cut,
        2, 3, 4, 5, 6, 6,
    )

    # Check that it decomposes properly to a lines-adjacency primitive
    prim = adj.decompose()
    assert isinstance(prim, core.GeomLinesAdjacency)
    verts = prim.get_vertex_list()
    assert tuple(verts) == (
        0, 0, 1, 2,

        0, 1, 2, 3,
        1, 2, 3, 4,

        2, 3, 4, 5,
        3, 4, 5, 6,
        4, 5, 6, 6,
    )
