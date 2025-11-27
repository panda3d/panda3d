/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file navMesh.h
 * @author Ashwani / 
 * @date 2024
 */

#ifndef NAVMESHPOLY_H
#define NAVMESHPOLY_H

#include "pandabase.h"
#include "config_navmesh.h" // Added config include
#include "typedReferenceCount.h"
#include "namable.h"
#include "luse.h"
#include "navPath.h"
#include "pointerTo.h"
#include "geomNode.h"

// Forward declare Detour types
class dtNavMesh;
class dtNavMeshQuery;

/**
 * This class represents a generated navigation mesh.
 * It holds the Detour mesh data and provides interfaces for pathfinding queries.
 * Renamed to NavMeshPoly to avoid conflict with legacy AI NavMesh typedef.
 */
class EXPCL_PANDA_NAVMESH NavMeshPoly : public TypedReferenceCount, public Namable {
PUBLISHED:
  NavMeshPoly();
  ~NavMeshPoly();

  // Finds a path from start to end points.
  // Returns a NavPath containing the waypoints, or an empty path if no path found.
  PT(NavPath) find_path(const LPoint3 &start, const LPoint3 &end);

  // Generates a visual representation of the navmesh for debugging.
  PT(GeomNode) make_debug_geom() const;

  // Writes the navmesh data to a BAM stream/file for caching.
  bool write_bam_stream(std::ostream &out) const;

public:
  // Internal method to setup the mesh from raw Detour data.
  // The NavMeshPoly takes ownership of the data buffer.
  void setup(unsigned char* data, int data_size);

private:
  dtNavMesh* _nav_mesh;
  dtNavMeshQuery* _nav_query;
  unsigned char* _nav_data;

public:
  static TypeHandle get_class_type() {
    return _type_handle;
  }
  static void init_type() {
    TypedReferenceCount::init_type();
    register_type(_type_handle, "NavMeshPoly",
                  TypedReferenceCount::get_class_type());
  }
  virtual TypeHandle get_type() const {
    return get_class_type();
  }
  virtual TypeHandle force_init_type() {init_type(); return get_class_type();}

private:
  static TypeHandle _type_handle;
};

#endif // NAVMESHPOLY_H
