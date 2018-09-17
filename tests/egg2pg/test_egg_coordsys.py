import pytest
from panda3d import core

# Skip these tests if we can't import egg.
egg = pytest.importorskip("panda3d.egg")


COORD_SYSTEMS = [core.CS_zup_right, core.CS_yup_right, core.CS_zup_left, core.CS_yup_left]


@pytest.mark.parametrize("egg_coordsys", COORD_SYSTEMS)
@pytest.mark.parametrize("coordsys", COORD_SYSTEMS)
def test_egg2pg_transform_ident(egg_coordsys, coordsys):
    # Ensures that an identity matrix always remains untouched.
    group = egg.EggGroup("group")
    group.add_matrix4(core.Mat4D.ident_mat())
    assert group.transform_is_identity()

    assert group.get_vertex_frame() == core.Mat4D.ident_mat()
    assert group.get_node_frame() == core.Mat4D.ident_mat()
    assert group.get_vertex_frame_inv() == core.Mat4D.ident_mat()
    assert group.get_node_frame_inv() == core.Mat4D.ident_mat()
    assert group.get_vertex_to_node() == core.Mat4D.ident_mat()
    assert group.get_node_to_vertex() == core.Mat4D.ident_mat()

    data = egg.EggData()
    data.set_coordinate_system(egg_coordsys)
    data.add_child(group)

    root = egg.load_egg_data(data, coordsys)
    assert root
    node, = root.children

    assert node.transform.is_identity()


@pytest.mark.parametrize("coordsys", COORD_SYSTEMS)
def test_egg2pg_transform_mat_unchanged(coordsys):
    # Ensures that the matrix remains unchanged if coordinate system is same.
    matv = (5, 2, -3, 4, 5, 6, 7, 8, 9, 1, -3, 2, 5, 2, 5, 2)
    mat = core.Mat4D(*matv)
    group = egg.EggGroup("group")
    group.add_matrix4(mat)
    assert not group.transform_is_identity()

    assert group.get_vertex_frame() == core.Mat4D.ident_mat()
    assert group.get_node_frame() == mat
    assert group.get_vertex_frame_inv() == core.Mat4D.ident_mat()
    assert group.get_node_frame_inv() == core.invert(mat)
    assert group.get_vertex_to_node() == core.invert(mat)
    assert group.get_node_to_vertex() == mat

    data = egg.EggData()
    data.set_coordinate_system(coordsys)
    data.add_child(group)

    root = egg.load_egg_data(data, coordsys)
    assert root
    node, = root.children

    assert node.transform.mat == core.Mat4(*matv)


@pytest.mark.parametrize("egg_coordsys", COORD_SYSTEMS)
@pytest.mark.parametrize("coordsys", COORD_SYSTEMS)
def test_egg2pg_transform_pos3d(egg_coordsys, coordsys):
    vpool = egg.EggVertexPool("vpool")
    vtx = vpool.make_new_vertex(core.Point3D.rfu(-8, 0.5, 4.5, egg_coordsys))

    point = egg.EggPoint()
    point.add_vertex(vtx)

    group = egg.EggGroup("group")
    group.add_translate3d(core.Point3D.rfu(1, 2, 3, egg_coordsys))
    assert not group.transform_is_identity()
    group.add_child(point)

    mat = group.get_transform3d()
    assert group.get_vertex_frame() == core.Mat4D.ident_mat()
    assert group.get_node_frame() == mat
    assert group.get_vertex_frame_inv() == core.Mat4D.ident_mat()
    assert group.get_node_frame_inv() == core.invert(mat)
    assert group.get_vertex_to_node() == core.invert(mat)
    assert group.get_node_to_vertex() == mat

    assert group.get_vertex_frame_ptr() is None
    assert group.get_vertex_frame_inv_ptr() is None

    data = egg.EggData()
    data.set_coordinate_system(egg_coordsys)
    data.add_child(vpool)
    data.add_child(group)

    root = egg.load_egg_data(data, coordsys)
    assert root
    node, = root.children

    # Ensure the node has the expected position.
    assert node.transform.pos == core.Point3.rfu(1, 2, 3, coordsys)

    # Get the location of the vertex.  This is a quick, hacky way to get it.
    point = core.NodePath(node).get_tight_bounds()[0]
    assert point == core.Point3.rfu(-8, 0.5, 4.5, coordsys)

