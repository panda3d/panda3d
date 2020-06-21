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

#ifndef NAVMESHNODE_H
#define NAVMESHNODE_H

#include <string>
#include "geom.h"
#include "pandaFramework.h"
#include "pandaSystem.h"
#include "navMesh.h"
#include <string>

class NavMeshNode: public PandaNode
{
PUBLISHED:
  NavMeshNode(const std::string &name, PT(NavMesh) nav_mesh);
private:
  PT(NavMesh) _nav_mesh;

public:
  
  ~NavMeshNode();
};

#endif // NAVMESHNODE_H
