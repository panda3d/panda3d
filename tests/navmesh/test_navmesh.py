import pytest
from panda3d.core import (
    Geom, GeomNode, GeomVertexData, GeomVertexFormat, GeomVertexWriter,
    GeomTriangles, NodePath, NavMeshSettings, NavMeshBuilder
)

@pytest.fixture
def simple_quad_geom():
    """Creates a simple quad geometry (2 triangles) on the X-Y plane."""
    format = GeomVertexFormat.get_v3()
    vdata = GeomVertexData('quad', format, Geom.UH_static)
    
    vertex = GeomVertexWriter(vdata, 'vertex')
    vertex.add_data3(0, 0, 0)
    vertex.add_data3(10, 0, 0)
    vertex.add_data3(10, 10, 0)
    vertex.add_data3(0, 10, 0)
    
    tris = GeomTriangles(Geom.UH_static)
    tris.add_vertices(0, 1, 2)
    tris.add_vertices(0, 2, 3)
    
    geom = Geom(vdata)
    geom.add_primitive(tris)
    
    node = GeomNode('quad_node')
    node.add_geom(geom)
    return NodePath(node)

def test_navmesh_settings():
    settings = NavMeshSettings()
    settings.set_cell_size(0.5)
    assert settings.get_cell_size() == 0.5
    
    settings.set_agent_radius(1.0)
    assert settings.get_agent_radius() == 1.0

def test_navmesh_builder_basic(simple_quad_geom):
    settings = NavMeshSettings()
    builder = NavMeshBuilder()
    
    # Should return a NavMesh object (even if empty for now as it's a stub)
    navmesh = builder.build(simple_quad_geom, settings)
    assert navmesh is not None

def test_navmesh_builder_empty():
    settings = NavMeshSettings()
    builder = NavMeshBuilder()
    
    empty_np = NodePath("empty")
    navmesh = builder.build(empty_np, settings)
    # The builder should handle empty input gracefully, possibly returning None or empty mesh
    # Our current C++ stub logs error and returns nullptr
    assert navmesh is None

