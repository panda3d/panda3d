// Filename: physxBoxControllerDesc.cxx
// Created by:  enn0x (22Sep09)
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

#include "physxBoxControllerDesc.h"
#include "physxManager.h"

////////////////////////////////////////////////////////////////////
//     Function: PhysxBoxControllerDesc::set_extents
//       Access: Published
//  Description: Sets the dimensions of the box.
//
//               The dimensions are the 'radii' of the box,
//               meaning 1/2 extents in x dimension, 1/2 extents
//               in y dimension, 1/2 extents in z dimension.
////////////////////////////////////////////////////////////////////
void PhysxBoxControllerDesc::
set_extents(const LVector3f &extents) {

  _desc.extents = PhysxManager::vec3_to_nxVec3(extents);
}

////////////////////////////////////////////////////////////////////
//     Function: PhysxBoxControllerDesc::get_extents
//       Access: Published
//  Description: Returns the dimensions of the box.
////////////////////////////////////////////////////////////////////
LVector3f PhysxBoxControllerDesc::
get_extents() const {

  return PhysxManager::nxVec3_to_vec3(_desc.extents);
}

