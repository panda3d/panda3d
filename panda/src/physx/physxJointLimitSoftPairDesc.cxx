// Filename: physxJointLimitSoftPairDesc.cxx
// Created by:  pratt (Jun 20, 2006)
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

#ifdef HAVE_PHYSX

#include "physxJointLimitSoftPairDesc.h"

#include "physxJointLimitSoftDesc.h"


////////////////////////////////////////////////////////////////////
//     Function : PhysxJointLimitSoftPairDesc
//       Access : Published
//  Description :
////////////////////////////////////////////////////////////////////
PhysxJointLimitSoftPairDesc::
PhysxJointLimitSoftPairDesc() {

}

////////////////////////////////////////////////////////////////////
//     Function : get_high
//       Access : Published
//  Description :
////////////////////////////////////////////////////////////////////
PhysxJointLimitSoftDesc & PhysxJointLimitSoftPairDesc::
get_high() const {
  throw "Not Implemented"; // return nJointLimitSoftPairDesc.high;
}

////////////////////////////////////////////////////////////////////
//     Function : get_low
//       Access : Published
//  Description :
////////////////////////////////////////////////////////////////////
PhysxJointLimitSoftDesc & PhysxJointLimitSoftPairDesc::
get_low() const {
  throw "Not Implemented"; // return nJointLimitSoftPairDesc.low;
}

////////////////////////////////////////////////////////////////////
//     Function : set_high
//       Access : Published
//  Description :
////////////////////////////////////////////////////////////////////
void PhysxJointLimitSoftPairDesc::
set_high(PhysxJointLimitSoftDesc & value) {
  nJointLimitSoftPairDesc.high = value.nJointLimitSoftDesc;
}

////////////////////////////////////////////////////////////////////
//     Function : set_low
//       Access : Published
//  Description :
////////////////////////////////////////////////////////////////////
void PhysxJointLimitSoftPairDesc::
set_low(PhysxJointLimitSoftDesc & value) {
  nJointLimitSoftPairDesc.low = value.nJointLimitSoftDesc;
}

#endif // HAVE_PHYSX

