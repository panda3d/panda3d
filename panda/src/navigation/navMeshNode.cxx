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


#include "navMeshNode.h"


NavMeshNode::NavMeshNode(const std::string &name, PT(NavMesh) nav_mesh):
  PandaNode(name)
{
  _nav_mesh = nav_mesh;
}

NavMeshNode::~NavMeshNode() {
  
}


