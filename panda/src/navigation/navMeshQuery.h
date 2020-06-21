/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file movieAudio.h
 * @author ashwini
 * @date 2020-060-21
 */


#ifndef NAVMESHQUERY_H
#define NAVMESHQUERY_H

#include "DetourNavMeshQuery.h"
#include "navMeshBuilder.h"

class NavMeshQuery
{
PUBLISHED:
  void set_nav_query(NavMeshBuilder *nav_mesh) { _nav_query = nav_mesh->get_nav_query(); }
  bool nearest_point(LPoint3 &p);
private:
  class dtNavMeshQuery *_nav_query;
  
public:
  
  
};

#endif // NAVMESHQUERY_H
