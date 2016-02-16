/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file physxJointLimitDesc.cxx
 * @author enn0x
 * @date 2009-09-28
 */

#include "physxJointLimitDesc.h"

////////////////////////////////////////////////////////////////////
//     Function: PhysxJointLimitDesc::set_value
//       Access: Published
//  Description: 
////////////////////////////////////////////////////////////////////
void PhysxJointLimitDesc::
set_value(float value) {

  _desc.value = value;
}

////////////////////////////////////////////////////////////////////
//     Function: PhysxJointLimitDesc::set_restitution
//       Access: Published
//  Description: 
////////////////////////////////////////////////////////////////////
void PhysxJointLimitDesc::
set_restitution(float restitution) {

  _desc.restitution = restitution;
}

////////////////////////////////////////////////////////////////////
//     Function: PhysxJointLimitDesc::set_hardness
//       Access: Published
//  Description: 
////////////////////////////////////////////////////////////////////
void PhysxJointLimitDesc::
set_hardness(float hardness) {

  _desc.hardness = hardness;
}

////////////////////////////////////////////////////////////////////
//     Function: PhysxJointLimitDesc::get_value
//       Access: Published
//  Description: 
////////////////////////////////////////////////////////////////////
float PhysxJointLimitDesc::
get_value() const {

  return _desc.value;
}

////////////////////////////////////////////////////////////////////
//     Function: PhysxJointLimitDesc::get_restitution
//       Access: Published
//  Description: 
////////////////////////////////////////////////////////////////////
float PhysxJointLimitDesc::
get_restitution() const {

  return _desc.restitution;
}

////////////////////////////////////////////////////////////////////
//     Function: PhysxJointLimitDesc::get_hardness
//       Access: Published
//  Description: 
////////////////////////////////////////////////////////////////////
float PhysxJointLimitDesc::
get_hardness() const {

  return _desc.hardness;
}

