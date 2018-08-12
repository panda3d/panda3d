import pytest
from panda3d import core

# Skip these tests if we can't import egg.
egg = pytest.importorskip("panda3d.egg")


EGG_TRANSFORM_MISSING = """
<CoordinateSystem> { %s }
<Group> {
}
"""

EGG_TRANSFORM_EMPTY = """
<CoordinateSystem> { %s }
<Group> {
    <Transform> {
    }
}
"""

EGG_TRANSFORM_IDENT = """
<CoordinateSystem> { %s }
<Group> {
    <Transform> {
        <Matrix4> {
            1 0 0 0
            0 1 0 0
            0 0 1 0
            0 0 0 1
        }
    }
}
"""

EGG_TRANSFORM_MATRIX = """
<CoordinateSystem> { %s }
<Group> {
    <Transform> {
        <Matrix4> {
            5 2 -3 4
            5 6 7 8
            9 1 -3 2
            5 2 5 2
        }
    }
}
"""

COORD_SYSTEMS = {
    core.CS_zup_right: "zup-right",
    core.CS_yup_right: "yup-right",
    core.CS_zup_left: "zup-left",
    core.CS_yup_left: "yup-left",
}

def read_egg_string(string):
    """Reads an EggData from a string."""
    stream = core.StringStream(string.encode('utf-8'))
    data = egg.EggData()
    assert data.read(stream)
    return data


@pytest.mark.parametrize("coordsys", COORD_SYSTEMS.keys())
def test_egg_transform_missing(coordsys):
    data = read_egg_string(EGG_TRANSFORM_MISSING % COORD_SYSTEMS[coordsys])
    assert data.get_coordinate_system() == coordsys

    child, = data.get_children()
    assert not child.has_transform3d()
    assert child.transform_is_identity()

    assert child.get_vertex_frame() == core.Mat4D.ident_mat()
    assert child.get_node_frame() == core.Mat4D.ident_mat()
    assert child.get_vertex_frame_inv() == core.Mat4D.ident_mat()
    assert child.get_node_frame_inv() == core.Mat4D.ident_mat()
    assert child.get_vertex_to_node() == core.Mat4D.ident_mat()
    assert child.get_node_to_vertex() == core.Mat4D.ident_mat()


@pytest.mark.parametrize("coordsys", COORD_SYSTEMS.keys())
def test_egg_transform_empty(coordsys):
    data = read_egg_string(EGG_TRANSFORM_EMPTY % COORD_SYSTEMS[coordsys])
    assert data.get_coordinate_system() == coordsys

    child, = data.get_children()
    assert not child.has_transform3d()
    assert child.transform_is_identity()

    assert child.get_vertex_frame() == core.Mat4D.ident_mat()
    assert child.get_node_frame() == core.Mat4D.ident_mat()
    assert child.get_vertex_frame_inv() == core.Mat4D.ident_mat()
    assert child.get_node_frame_inv() == core.Mat4D.ident_mat()
    assert child.get_vertex_to_node() == core.Mat4D.ident_mat()
    assert child.get_node_to_vertex() == core.Mat4D.ident_mat()


@pytest.mark.parametrize("coordsys", COORD_SYSTEMS.keys())
def test_egg_transform_ident(coordsys):
    data = read_egg_string(EGG_TRANSFORM_IDENT % COORD_SYSTEMS[coordsys])
    assert data.get_coordinate_system() == coordsys

    child, = data.get_children()
    assert child.has_transform3d()
    assert child.transform_is_identity()

    assert child.get_vertex_frame() == core.Mat4D.ident_mat()
    assert child.get_node_frame() == core.Mat4D.ident_mat()
    assert child.get_vertex_frame_inv() == core.Mat4D.ident_mat()
    assert child.get_node_frame_inv() == core.Mat4D.ident_mat()
    assert child.get_vertex_to_node() == core.Mat4D.ident_mat()
    assert child.get_node_to_vertex() == core.Mat4D.ident_mat()


@pytest.mark.parametrize("coordsys", COORD_SYSTEMS.keys())
def test_egg_transform_matrix(coordsys):
    data = read_egg_string(EGG_TRANSFORM_MATRIX % COORD_SYSTEMS[coordsys])
    assert data.get_coordinate_system() == coordsys

    mat = core.Mat4D(5, 2, -3, 4, 5, 6, 7, 8, 9, 1, -3, 2, 5, 2, 5, 2)
    mat_inv = core.invert(mat)

    child, = data.get_children()
    assert child.has_transform3d()
    assert not child.transform_is_identity()
    assert child.get_transform3d() == mat

    assert child.get_vertex_frame() == core.Mat4D.ident_mat()
    assert child.get_node_frame() == mat
    assert child.get_vertex_frame_inv() == core.Mat4D.ident_mat()
    assert child.get_node_frame_inv() == mat_inv
    assert child.get_vertex_to_node() == mat_inv
    assert child.get_node_to_vertex() == mat
