/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file navMesh.cxx
 * @author Ashwani / 
 * @date 2024
 */

#include "navMesh.h"
#include "config_navmesh.h"
#include "geomNode.h"
#include "geomLines.h"
#include "geomTriangles.h"
#include "geomVertexWriter.h"

#include <DetourNavMesh.h>
#include <DetourNavMeshQuery.h>

TypeHandle NavMeshPoly::_type_handle;

/**
 * Constructor
 */
NavMeshPoly::
NavMeshPoly() :
  _nav_mesh(nullptr),
  _nav_query(nullptr),
  _nav_data(nullptr)
{
  _nav_mesh = dtAllocNavMesh();
  _nav_query = dtAllocNavMeshQuery();
}

/**
 * Destructor
 */
NavMeshPoly::
~NavMeshPoly() {
  if (_nav_query) {
    dtFreeNavMeshQuery(_nav_query);
  }
  if (_nav_mesh) {
    dtFreeNavMesh(_nav_mesh);
  }
  // If dtNavMesh was initialized with DT_TILE_FREE_DATA, it frees the buffer.
  // Otherwise we must free it. Since we pass DT_TILE_FREE_DATA in setup(),
  // we do NOT free _nav_data here manually if setup() was called successfully.
  // However, if setup() was never called, _nav_data is null anyway.
}

/**
 * Internal method to setup the mesh from raw Detour data.
 */
void NavMeshPoly::
setup(unsigned char* data, int data_size) {
  _nav_data = data;
  dtStatus status = _nav_mesh->init(_nav_data, data_size, DT_TILE_FREE_DATA);
  if (dtStatusFailed(status)) {
    navmesh_cat.error() << "Could not init Detour navmesh\n";
    return;
  }
  
  status = _nav_query->init(_nav_mesh, 2048);
  if (dtStatusFailed(status)) {
    navmesh_cat.error() << "Could not init Detour navmesh query\n";
  }
}

/**
 * Finds a path from start to end points.
 */
PT(NavPath) NavMeshPoly::
find_path(const LPoint3 &start, const LPoint3 &end) {
  PT(NavPath) path = new NavPath();
  
  if (!_nav_query || !_nav_mesh) {
    return path;
  }

  // Convert points to Recast coordinate system (Z-up -> Y-up)
  // (x, y, z) -> (x, z, -y)
  float start_rc[3] = {start[0], start[2], -start[1]};
  float end_rc[3] = {end[0], end[2], -end[1]};

  const int MAX_POLYS = 256;
  dtPolyRef start_ref, end_ref;
  float start_pt[3], end_pt[3];
  float extents[3] = {2.0f, 4.0f, 2.0f}; // Default search extent
  dtQueryFilter filter; // Default filter

  // Find nearest poly to start
  _nav_query->findNearestPoly(start_rc, extents, &filter, &start_ref, start_pt);
  
  // Find nearest poly to end
  _nav_query->findNearestPoly(end_rc, extents, &filter, &end_ref, end_pt);

  if (!start_ref || !end_ref) {
    navmesh_cat.warning()
      << "NavMeshPoly::find_path: failed to find nearest polys (start_ref="
      << start_ref << ", end_ref=" << end_ref << ")\n";
    return path;
  }

  // Find path
  dtPolyRef path_polys[MAX_POLYS];
  int path_count = 0;
  dtStatus path_status = _nav_query->findPath(start_ref, end_ref, start_pt, end_pt,
                                              &filter, path_polys, &path_count, MAX_POLYS);
  if (dtStatusFailed(path_status)) {
    navmesh_cat.error() << "Detour findPath failed (status=" << path_status << ")\n";
    return path;
  }

  if (path_count == 0) {
    navmesh_cat.warning() << "Detour findPath returned 0 polygons\n";
    return path;
  }

  // Smooth path
  const int MAX_STRAIGHT_PATH = 256;
  float straight_path[MAX_STRAIGHT_PATH * 3];
  unsigned char straight_path_flags[MAX_STRAIGHT_PATH];
  dtPolyRef straight_path_refs[MAX_STRAIGHT_PATH];
  int num_straight = 0;

  dtStatus straight_status = _nav_query->findStraightPath(
      start_pt, end_pt, path_polys, path_count, straight_path,
      straight_path_flags, straight_path_refs, &num_straight, MAX_STRAIGHT_PATH);
  if (dtStatusFailed(straight_status)) {
    navmesh_cat.error() << "Detour findStraightPath failed (status=" << straight_status << ")\n";
    return path;
  }
  if (num_straight == 0) {
    navmesh_cat.warning() << "Detour findStraightPath returned zero waypoints\n";
    return path;
  }

  // Convert result back to Panda coordinate system (Y-up -> Z-up)
  // (x, y, z) -> (x, -z, y)
  for (int i = 0; i < num_straight; ++i) {
    float* p = &straight_path[i * 3];
    path->add_point(LPoint3(p[0], -p[2], p[1]));
  }

  if (path->get_num_points() == 0) {
    navmesh_cat.warning() << "NavMeshPoly::find_path returned no waypoints despite non-zero straight path count\n";
  }

  return path;
}

/**
 * Generates a visual representation of the navmesh for debugging.
 */
PT(GeomNode) NavMeshPoly::
make_debug_geom() const {
  PT(GeomNode) geom_node = new GeomNode("navmesh_debug");
  
  if (!_nav_mesh) {
    return geom_node;
  }

  // Create vertex data format
  PT(GeomVertexData) vdata = new GeomVertexData("debug", GeomVertexFormat::get_v3c4(), Geom::UH_static);
  GeomVertexWriter vertex(vdata, "vertex");
  GeomVertexWriter color(vdata, "color");

  PT(GeomLines) lines = new GeomLines(Geom::UH_static);

  // Iterate over all tiles
  for (int i = 0; i < _nav_mesh->getMaxTiles(); ++i) {
    const dtMeshTile* tile = ((const dtNavMesh*)_nav_mesh)->getTile(i);
    if (!tile || !tile->header) continue;

    for (int j = 0; j < tile->header->polyCount; ++j) {
      const dtPoly* poly = &tile->polys[j];
      if (poly->getType() == DT_POLYTYPE_OFFMESH_CONNECTION) continue;

      const dtPolyDetail* pd = &tile->detailMeshes[j];

      for (int k = 0; k < pd->triCount; ++k) {
        const unsigned char* t = &tile->detailTris[(pd->triBase + k) * 4];
        
        // Retrieve the 3 vertex positions first
        LPoint3 points[3];
        for (int m = 0; m < 3; ++m) {
           float* v;
           if (t[m] < poly->vertCount) {
              v = &tile->verts[poly->verts[t[m]] * 3];
           } else {
              v = &tile->detailVerts[(pd->vertBase + t[m] - poly->vertCount) * 3];
           }
           points[m].set(v[0], -v[2], v[1]);
        }
        
        // Now write 3 lines (6 verts) to ensure simple logic
        // 0-1
        vertex.add_data3f(points[0]); color.add_data4f(0, 1, 1, 1);
        vertex.add_data3f(points[1]); color.add_data4f(0, 1, 1, 1);
        lines->add_vertex(lines->get_num_vertices());
        lines->add_vertex(lines->get_num_vertices() + 1);
        
        // 1-2
        vertex.add_data3f(points[1]); color.add_data4f(0, 1, 1, 1);
        vertex.add_data3f(points[2]); color.add_data4f(0, 1, 1, 1);
        lines->add_vertex(lines->get_num_vertices());
        lines->add_vertex(lines->get_num_vertices() + 1);
        
        // 2-0
        vertex.add_data3f(points[2]); color.add_data4f(0, 1, 1, 1);
        vertex.add_data3f(points[0]); color.add_data4f(0, 1, 1, 1);
        lines->add_vertex(lines->get_num_vertices());
        lines->add_vertex(lines->get_num_vertices() + 1);
      }
    }
  }
  
  lines->close_primitive(); // Important!

  PT(Geom) geom = new Geom(vdata);
  geom->add_primitive(lines);
  geom_node->add_geom(geom);

  return geom_node;
}

/**
 * Writes the navmesh data to a BAM stream.
 */
bool NavMeshPoly::
write_bam_stream(std::ostream &out) const {
  // Not implemented in this phase
  return false;
}
