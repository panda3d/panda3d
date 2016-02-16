/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file physxSpringDesc.cxx
 * @author enn0x
 * @date 2009-09-28
 */

#include "physxSpringDesc.h"

////////////////////////////////////////////////////////////////////
//     Function: PhysxSpringDesc::set_spring
//       Access: Published
//  Description: 
////////////////////////////////////////////////////////////////////
void PhysxSpringDesc::
set_spring(float spring) {

  _desc.spring = spring;
}

////////////////////////////////////////////////////////////////////
//     Function: PhysxSpringDesc::set_damper
//       Access: Published
//  Description: 
////////////////////////////////////////////////////////////////////
void PhysxSpringDesc::
set_damper(float damper) {

  _desc.damper = damper;
}

////////////////////////////////////////////////////////////////////
//     Function: PhysxSpringDesc::set_target_value
//       Access: Published
//  Description: 
////////////////////////////////////////////////////////////////////
void PhysxSpringDesc::
set_target_value(float targetValue) {

  _desc.targetValue = targetValue;
}

////////////////////////////////////////////////////////////////////
//     Function: PhysxSpringDesc::get_spring
//       Access: Published
//  Description: 
////////////////////////////////////////////////////////////////////
float PhysxSpringDesc::
get_spring() const {

  return _desc.spring;
}

////////////////////////////////////////////////////////////////////
//     Function: PhysxSpringDesc::get_damper
//       Access: Published
//  Description: 
////////////////////////////////////////////////////////////////////
float PhysxSpringDesc::
get_damper() const {

  return _desc.damper;
}

////////////////////////////////////////////////////////////////////
//     Function: PhysxSpringDesc::get_target_value
//       Access: Published
//  Description: 
////////////////////////////////////////////////////////////////////
float PhysxSpringDesc::
get_target_value() const {

  return _desc.targetValue;
}

