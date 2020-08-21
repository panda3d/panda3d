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


def test_geom_calc_sphere_bounds():
    # Ensure that it ignores NaN
    data = core.GeomVertexData("", core.GeomVertexFormat.get_v3(), core.Geom.UH_static)
    vertex = core.GeomVertexWriter(data, "vertex")
    vertex.add_data3((float("NaN"), 0, 0))
    vertex.add_data3((1, 1, 1))
    vertex.add_data3((1, 1, 2))

    prim = core.GeomPoints(core.Geom.UH_static)
    prim.add_next_vertices(3)

    geom = core.Geom(data)
    geom.add_primitive(prim)
    geom.set_bounds_type(core.BoundingVolume.BT_sphere)

    bounds = geom.get_bounds()
    assert isinstance(bounds, core.BoundingSphere)
    assert bounds.get_center() == (1, 1, 1.5)
    assert bounds.get_radius() == 0.5


def test_geom_calc_box_bounds():
    # Ensure that it ignores NaN
    data = core.GeomVertexData("", core.GeomVertexFormat.get_v3(), core.Geom.UH_static)
    vertex = core.GeomVertexWriter(data, "vertex")
    vertex.add_data3((float("NaN"), 0, 0))
    vertex.add_data3((1, 1, 1))
    vertex.add_data3((1, 1, 2))

    prim = core.GeomPoints(core.Geom.UH_static)
    prim.add_next_vertices(3)

    geom = core.Geom(data)
    geom.add_primitive(prim)
    geom.set_bounds_type(core.BoundingVolume.BT_box)

    bounds = geom.get_bounds()
    assert isinstance(bounds, core.BoundingBox)
    assert bounds.get_min() == (1, 1, 1)
    assert bounds.get_max() == (1, 1, 2)
