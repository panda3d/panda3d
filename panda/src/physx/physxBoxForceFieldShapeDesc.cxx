// Filename: physxBoxForceFieldShapeDesc.cxx
// Created by:  enn0x (06Nov09)
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

#include "physxBoxForceFieldShapeDesc.h"
#include "physxManager.h"

////////////////////////////////////////////////////////////////////
//     Function: PhysxBoxForceFieldShapeDesc::set_dimensions
//       Access: Published
//  Description: Sets the dimensions of the box.
//
//               The dimensions are the 'radii' of the box,
//               meaning 1/2 extents in x dimension, 1/2 extents
//               in y dimension, 1/2 extents in z dimension.
////////////////////////////////////////////////////////////////////
void PhysxBoxForceFieldShapeDesc::
set_dimensions(const LVector3f &dimensions) {

  nassertv(!dimensions.is_nan());
  _desc.dimensions = PhysxManager::vec3_to_nxVec3(dimensions);
}

////////////////////////////////////////////////////////////////////
//     Function: PhysxBoxForceFieldShapeDesc::get_dimensions
//       Access: Published
//  Description: Returns the dimensions of the box.
////////////////////////////////////////////////////////////////////
LVector3f PhysxBoxForceFieldShapeDesc::
get_dimensions() const {

  return PhysxManager::nxVec3_to_vec3(_desc.dimensions);
}

