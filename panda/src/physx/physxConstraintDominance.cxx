// Filename: physxConstraintDominance.cxx
// Created by:  enn0x (22Dec09)
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

#include "physxConstraintDominance.h"

////////////////////////////////////////////////////////////////////
//     Function: PhysxConstraintDominance::get_0
//       Access: Published
//  Description: Retruns the first dominance factor.
////////////////////////////////////////////////////////////////////
float PhysxConstraintDominance::
get_0() const {

  return _dominance.dominance0;
}

////////////////////////////////////////////////////////////////////
//     Function: PhysxConstraintDominance::get_1
//       Access: Published
//  Description: Returns the second dominance factor.
////////////////////////////////////////////////////////////////////
float PhysxConstraintDominance::
get_1() const {

  return _dominance.dominance1;
}

////////////////////////////////////////////////////////////////////
//     Function: PhysxConstraintDominance::set_0
//       Access: Published
//  Description: Sets the first dominance factor.
////////////////////////////////////////////////////////////////////
void PhysxConstraintDominance::
set_0(float d0) {

  _dominance.dominance0 = d0;
}

////////////////////////////////////////////////////////////////////
//     Function: PhysxConstraintDominance::set_1
//       Access: Published
//  Description: Sets the second dominance factor.
////////////////////////////////////////////////////////////////////
void PhysxConstraintDominance::
set_1(float d1) {

  _dominance.dominance1 = d1;
}

