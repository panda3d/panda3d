/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file navMeshQuery.h
 * @author ashwini
 * @date 2020-060-21
 */


#ifndef NAVMESHQUERY_H
#define NAVMESHQUERY_H

#include "recastnavigation/DetourNavMeshQuery.h"
#include "navMesh.h"
#include "navMeshPath.h"
#include "navMeshQueryFilter.h"
#include "pta_LVecBase3.h"
#include "pandaSystem.h"
#include "nodePath.h"

/**
 * NavMeshQuery class contains the functions to query the navigation mesh stored 
 * in a NavMesh object. The queries include functions to find the paths from one 
 * point to another over the mesh.
 */
class EXPCL_NAVIGATION NavMeshQuery {
PUBLISHED:
  explicit NavMeshQuery(PT(NavMesh) nav_mesh, int max_nodes = 2048);
  explicit NavMeshQuery(NodePath nav_mesh_node_path, int max_nodes = 2048);

  LPoint3 nearest_point(LPoint3 p, LVector3 extents = LVector3( 10 , 10 , 10 ));
  NavMeshPath find_path(LPoint3 &start, LPoint3 &end, LVector3 extents = LVector3( 10 , 10 , 10 ));
  NavMeshPath find_smooth_path(LPoint3 &start, LPoint3 &end, LVector3 extents = LVector3( 10 , 10 , 10 ));
  NavMeshPath find_straight_path(LPoint3 &start, LPoint3 &end, LVector3 extents = LVector3( 10 , 10 , 10 ), int opt = 0);

  NavMeshQueryFilter get_filter() const;
  void set_filter(NavMeshQueryFilter &filter);
  MAKE_PROPERTY(filter, get_filter, set_filter);

private:
  dtNavMeshQuery *_nav_query;
  static const int MAX_POLYS = 256;
  NavMeshQueryFilter _filter;

  LMatrix4 mat_from_y = LMatrix4::convert_mat(CS_yup_right, CS_default);
  LMatrix4 mat_to_y = LMatrix4::convert_mat(CS_default, CS_yup_right);

public:
  
  ~NavMeshQuery();
  
};

#include "navMeshQuery.I"

#endif // NAVMESHQUERY_H
