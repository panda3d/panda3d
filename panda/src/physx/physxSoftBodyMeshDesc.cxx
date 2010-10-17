// Filename: physxSoftBodyMeshDesc.cxx
// Created by:  enn0x (12Sep10)
//
////////////////////////////////////////////////////////////////////
//
// PANDA 3D SOFTWARE
// Copyright (c) Carnegie Mellon University.  All rights reserved.
//
// All use of this software is subject to the terms of the revised BSD
// license.  You should have received a copy of this license along
// with this source code in a file named "LICENSE."
//
////////////////////////////////////////////////////////////////////

#include "physxSoftBodyMeshDesc.h"
#include "physxManager.h"

////////////////////////////////////////////////////////////////////
//     Function: PhysxSoftBodyMeshDesc::set_num_vertices
//       Access: Published
//  Description: Sets the number of vertices to be stored within
//               this soft body mesh. The function allocates memory
//               for the vertices, but it does not set any vertices.
//
//               This method must be called before any calls to
//               set_vertex are done!
////////////////////////////////////////////////////////////////////
void PhysxSoftBodyMeshDesc::
set_num_vertices(unsigned int numVertices) {

  // Vertices
  if (_desc.vertices) {
    delete [] _vertices;
  }

  _vertices = new NxVec3[numVertices];

  _desc.numVertices = numVertices;
  _desc.vertices = _vertices;
}

////////////////////////////////////////////////////////////////////
//     Function: PhysxSoftBodyMeshDesc::set_vertex
//       Access: Published
//  Description: Sets a single vertex. You have to call the function
//               set_num_vertices before you can call this function.
////////////////////////////////////////////////////////////////////
void PhysxSoftBodyMeshDesc::
set_vertex(unsigned int idx, const LPoint3f &vert) {

  nassertv(_desc.numVertices > idx);

  _vertices[idx] = PhysxManager::point3_to_nxVec3(vert);
}

////////////////////////////////////////////////////////////////////
//     Function: PhysxSoftBodyMeshDesc::set_num_tetrahedra
//       Access: Published
//  Description: Sets the number of tetrahedra to be stored in this
//               soft body mesh.
//
//               This method must be called before any calls to
//               set_tetrahedron are done!
////////////////////////////////////////////////////////////////////
void PhysxSoftBodyMeshDesc::
set_num_tetrahedra(unsigned int numTetrahedra) {

  if (_desc.tetrahedra) {
    delete [] _tetrahedra;
  }

  _tetrahedra = new NxU32[4 * numTetrahedra];

  _desc.numTetrahedra = numTetrahedra;
  _desc.tetrahedra = _tetrahedra;
}

////////////////////////////////////////////////////////////////////
//     Function: PhysxSoftBodyMeshDesc::set_tetrahedron
//       Access: Published
//  Description: Sets a single tetrahedron, by providing the three
//               indices i1, i2, i3, i4.
////////////////////////////////////////////////////////////////////
void PhysxSoftBodyMeshDesc::
set_tetrahedron(unsigned int idx,
                unsigned int i1, unsigned int i2, unsigned int i3, unsigned int i4) {

  nassertv(_desc.numTetrahedra > idx);

  idx = 4 * idx;
  _tetrahedra[idx]     = i1;
  _tetrahedra[idx + 1] = i2;
  _tetrahedra[idx + 2] = i3;
  _tetrahedra[idx + 3] = i4;
}

