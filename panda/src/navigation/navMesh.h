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


#ifndef NAVMESH_H
#define NAVMESH_H

#include "Recast.h"
#include "DetourNavMesh.h"
#include "pandaFramework.h"

class NavMesh: public TypedWritableReferenceCount
{
PUBLISHED:
  NavMesh(dtNavMesh *nav_mesh);
  void set_nav_mesh(dtNavMesh *m) { _nav_mesh = m; }

private:
  dtNavMesh *_nav_mesh;
  
public:
  NavMesh();
  
  ~NavMesh();
  
};

#endif // NAVMESH_H
