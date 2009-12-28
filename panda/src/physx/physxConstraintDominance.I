// Filename: physxConstraintDominance.I
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


////////////////////////////////////////////////////////////////////
//     Function: PhysxConstraintDominance::Constructor
//       Access: Published
//  Description:
////////////////////////////////////////////////////////////////////
INLINE PhysxConstraintDominance::
PhysxConstraintDominance(float d0, float d1) : _dominance(d0, d1) {

}

////////////////////////////////////////////////////////////////////
//     Function: PhysxConstraintDominance::Destructor
//       Access: Published
//  Description:
////////////////////////////////////////////////////////////////////
INLINE PhysxConstraintDominance::
~PhysxConstraintDominance() {

}

////////////////////////////////////////////////////////////////////
//     Function: PhysxConstraintDominance::get_dominance
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
INLINE NxConstraintDominance PhysxConstraintDominance::
get_dominance() const {

  return _dominance;
}

////////////////////////////////////////////////////////////////////
//     Function: PhysxConstraintDominance::set_dominance
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
INLINE void PhysxConstraintDominance::
set_dominance(NxConstraintDominance value) {

  _dominance = value;
}

