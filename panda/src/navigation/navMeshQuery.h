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

#include "DetourNavMeshQuery.h"
#include "navMesh.h"
#include "pta_LVecBase3.h"
#include "pandaFramework.h"
#include "pandaSystem.h"
#include "nodePath.h"

class EXPCL_NAVIGATION NavMeshQuery
{

PUBLISHED:
  
  NavMeshQuery();
  NavMeshQuery(PT(NavMesh) nav_mesh);
  bool set_nav_query(PT(NavMesh) nav_mesh);
  NavMeshQuery(NodePath nav_mesh_node_path);
  bool set_nav_query(NodePath nav_mesh_node_path);
  bool nearest_point(LPoint3 &p);
  PTA_LVecBase3 find_path(LPoint3 &start, LPoint3 &end);
  PTA_LVecBase3 find_straight_path(LPoint3 &start, LPoint3 &end, int opt=0);

private:
  
  class dtNavMeshQuery *_nav_query;
  static const int MAX_POLYS = 256;

public:
  
  ~NavMeshQuery();
  
  
};

#endif // NAVMESHQUERY_H
