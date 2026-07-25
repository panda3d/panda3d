"""Regression coverage for the Panda3D navmesh module.

This file intentionally focuses on end-to-end smoke tests so that we can
verify the C++ NavMeshBuilder → NavMeshPoly → NavPath pipeline via the
Python bindings exposed through interrogate.
"""

import unittest

from panda3d.core import (
    Geom, GeomNode, GeomTriangles, GeomVertexData, GeomVertexFormat,
    GeomVertexWriter, LPoint3, NodePath
)
from panda3d.navmesh import NavMeshBuilder, NavMeshPoly, NavMeshSettings


def _make_simple_geometry():
    """Create a 20x20 quad (two triangles) centered at the origin."""
    vdata = GeomVertexData("quad", GeomVertexFormat.get_v3(), Geom.UH_static)
    vertex = GeomVertexWriter(vdata, "vertex")

    corners = [
        (-10, -10, 0),
        (10, -10, 0),
        (10, 10, 0),
        (-10, 10, 0),
    ]
    for corner in corners:
        vertex.add_data3f(*corner)

    tris = GeomTriangles(Geom.UH_static)
    tris.add_vertices(0, 1, 2)
    tris.add_vertices(0, 2, 3)

    geom = Geom(vdata)
    geom.add_primitive(tris)

    node = GeomNode("quad_node")
    node.add_geom(geom)
    return NodePath(node)


class NavMeshBasicTests(unittest.TestCase):
    """Minimal smoke tests that confirm navmesh creation and querying."""

    def setUp(self):
        self.settings = NavMeshSettings()

    def test_settings_validation(self):
        """NavMeshSettings::validate should guard against invalid inputs."""
        self.assertTrue(self.settings.validate())
        self.settings.set_cell_size(-1)
        self.assertFalse(self.settings.validate())

    def test_builder_success(self):
        """A simple quad should produce a NavMeshPoly instance."""
        mesh = NavMeshBuilder.build(_make_simple_geometry(), self.settings)
        self.assertIsNotNone(mesh)
        self.assertIsInstance(mesh, NavMeshPoly)

    def test_builder_failure_empty(self):
        """An empty NodePath should fail gracefully."""
        mesh = NavMeshBuilder.build(NodePath("empty"), self.settings)
        self.assertIsNone(mesh)

    def test_pathfinding_straight(self):
        """Paths within the quad should roughly preserve the requested points."""
        mesh = NavMeshBuilder.build(_make_simple_geometry(), self.settings)
        start = LPoint3(-5, -5, 0)
        end = LPoint3(5, 5, 0)

        path = mesh.find_path(start, end)
        self.assertIsNotNone(path)
        self.assertGreaterEqual(path.get_num_points(), 2)

        p_start = path.get_point(0)
        p_end = path.get_point(path.get_num_points() - 1)
        self.assertLess((p_start - start).length(), 1.0)
        self.assertLess((p_end - end).length(), 1.0)

    def test_pathfinding_off_mesh(self):
        """Extremely distant requests should clamp onto the mesh boundary."""
        mesh = NavMeshBuilder.build(_make_simple_geometry(), self.settings)
        start = LPoint3(-50, -50, 0)
        end = LPoint3(50, 50, 0)

        path = mesh.find_path(start, end)
        if path.get_num_points() > 0:
            p_start = path.get_point(0)
            self.assertLess(abs(p_start.x), 12.0)
            self.assertLess(abs(p_start.y), 12.0)

    def test_debug_geom(self):
        """The debug geom helper should emit non-empty geometry."""
        mesh = NavMeshBuilder.build(_make_simple_geometry(), self.settings)
        debug_node = mesh.make_debug_geom()
        self.assertIsNotNone(debug_node)
        self.assertIsInstance(debug_node, GeomNode)
        self.assertGreater(debug_node.get_num_geoms(), 0)

        geom = debug_node.get_geom(0)
        vdata = geom.get_vertex_data()
        self.assertGreater(vdata.get_num_rows(), 0)


if __name__ == "__main__":
    unittest.main()
