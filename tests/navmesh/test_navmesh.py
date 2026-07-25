"""Legacy regression coverage for the higher-level navmesh API surface."""

import unittest

from panda3d.core import (
    Geom, GeomNode, GeomTriangles, GeomVertexData, GeomVertexFormat,
    GeomVertexWriter, NodePath
)
from panda3d.navmesh import NavMeshBuilder, NavMeshPoly, NavMeshSettings


def _simple_quad_geom():
    """Build a basic 10x10 quad for NavMeshBuilder smoke tests."""
    vformat = GeomVertexFormat.get_v3()
    vdata = GeomVertexData("quad", vformat, Geom.UH_static)
    vertex = GeomVertexWriter(vdata, "vertex")
    vertex.add_data3(0, 0, 0)
    vertex.add_data3(10, 0, 0)
    vertex.add_data3(10, 10, 0)
    vertex.add_data3(0, 10, 0)

    tris = GeomTriangles(Geom.UH_static)
    tris.add_vertices(0, 1, 2)
    tris.add_vertices(0, 2, 3)

    geom = Geom(vdata)
    geom.add_primitive(tris)

    node = GeomNode("quad_node")
    node.add_geom(geom)
    return NodePath(node)


class NavMeshBuilderTests(unittest.TestCase):
    """Lightweight validation for settings and builder error handling."""

    def setUp(self):
        self.settings = NavMeshSettings()

    def test_navmesh_settings_roundtrip(self):
        """Getter/setter pairs should round-trip values exactly."""
        self.settings.set_cell_size(0.5)
        self.assertEqual(self.settings.get_cell_size(), 0.5)

        self.settings.set_agent_radius(1.0)
        self.assertEqual(self.settings.get_agent_radius(), 1.0)

    def test_navmesh_builder_basic(self):
        """A simple quad should produce a NavMeshPoly."""
        navmesh = NavMeshBuilder.build(_simple_quad_geom(), self.settings)
        self.assertIsNotNone(navmesh)
        self.assertIsInstance(navmesh, NavMeshPoly)

    def test_navmesh_builder_empty(self):
        """Empty input should fail without crashing."""
        navmesh = NavMeshBuilder.build(NodePath("empty"), self.settings)
        self.assertIsNone(navmesh)


if __name__ == "__main__":
    unittest.main()

