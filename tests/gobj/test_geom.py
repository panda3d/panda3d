from panda3d import core

empty_format = core.GeomVertexFormat.get_empty()


def test_geom_decompose_in_place():
    vertex_data = core.GeomVertexData("", empty_format, core.GeomEnums.UH_static)
    prim = core.GeomTristrips(core.GeomEnums.UH_static)
    prim.add_vertex(0)
    prim.add_vertex(1)
    prim.add_vertex(2)
    prim.add_vertex(3)
    prim.close_primitive()

    geom = core.Geom(vertex_data)
    geom.add_primitive(prim)

    geom.decompose_in_place()

    prim = geom.get_primitive(0)
    assert tuple(prim.get_vertex_list()) == (0, 1, 2, 2, 1, 3)


def test_geom_decompose():
    vertex_data = core.GeomVertexData("", empty_format, core.GeomEnums.UH_static)
    prim = core.GeomTristrips(core.GeomEnums.UH_static)
    prim.add_vertex(0)
    prim.add_vertex(1)
    prim.add_vertex(2)
    prim.add_vertex(3)
    prim.close_primitive()

    geom = core.Geom(vertex_data)
    geom.add_primitive(prim)

    new_geom = geom.decompose()

    new_prim = new_geom.get_primitive(0)
    assert tuple(new_prim.get_vertex_list()) == (0, 1, 2, 2, 1, 3)

    # Old primitive should still be unchanged
    assert prim == geom.get_primitive(0)
