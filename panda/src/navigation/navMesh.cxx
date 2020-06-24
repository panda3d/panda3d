/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file navMesh.cxx
 * @author ashwini
 * @date 2020-060-21
 */


#include "navMesh.h"
#include "geom.h"
#include "geomTrifans.h"

TypedWritableReferenceCount NavMesh::_type_handle;

NavMesh::NavMesh() {

}

NavMesh::~NavMesh() {
  
}

NavMesh::NavMesh(dtNavMesh *nav_mesh) {
  _nav_mesh = nav_mesh;
}

