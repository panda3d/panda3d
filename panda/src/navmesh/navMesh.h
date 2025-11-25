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

#ifndef NAVMESH_H
#define NAVMESH_H

#include "pandabase.h"
#include "typedReferenceCount.h"
#include "namable.h"
#include "luse.h"
#include "navPath.h"

/**
 * This class represents a generated navigation mesh.
 * It holds the Detour mesh data and provides interfaces for pathfinding queries.
 */
class EXPCL_PANDA_NAVMESH NavMesh : public TypedReferenceCount, public Namable {
PUBLISHED:
  NavMesh();
  ~NavMesh();

  // Finds a path from start to end points.
  // Returns a NavPath containing the waypoints, or an empty path if no path found.
  PT(NavPath) find_path(const LPoint3 &start, const LPoint3 &end);

  // Writes the navmesh data to a BAM stream/file for caching.
  bool write_bam_stream(std::ostream &out) const;

private:
  // Placeholder for Detour mesh data
  // dtNavMesh* _mesh; 
  // dtNavMeshQuery* _query;

public:
  static TypeHandle get_class_type() {
    return _type_handle;
  }
  static void init_type() {
    TypedReferenceCount::init_type();
    register_type(_type_handle, "NavMesh",
                  TypedReferenceCount::get_class_type());
  }
  virtual TypeHandle get_type() const {
    return get_class_type();
  }
  virtual TypeHandle force_init_type() {init_type(); return get_class_type();}

private:
  static TypeHandle _type_handle;
};

#endif // NAVMESH_H

