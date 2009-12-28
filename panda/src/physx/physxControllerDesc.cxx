// Filename: physxControllerDesc.cxx
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

#include "physxControllerDesc.h"
#include "physxManager.h"

////////////////////////////////////////////////////////////////////
//     Function: PhysxControllerDesc::set_pos
//       Access: Published
//  Description: Set the position of the character.
////////////////////////////////////////////////////////////////////
void PhysxControllerDesc::
set_pos(const LPoint3f &pos) {

  ptr()->position = PhysxManager::point3_to_nxExtVec3(pos);
}

////////////////////////////////////////////////////////////////////
//     Function: PhysxControllerDesc::set_slope_limit
//       Access: Published
//  Description: Sets the maximum slope which the character can walk
//               up. In general it is desirable to limit where the
//               character can walk, in particular it is unrealistic
//               for the character to be able to climb arbitary
//               slopes.
//               The value is expressed in degrees.
//               Default: 45.0 degrees.
////////////////////////////////////////////////////////////////////
void PhysxControllerDesc::
set_slope_limit(float slopeLimit) {

  ptr()->slopeLimit = cosf(NxMath::degToRad(slopeLimit));
}

////////////////////////////////////////////////////////////////////
//     Function: PhysxControllerDesc::set_skin_width
//       Access: Published
//  Description: Sets the skin width used by the controller. 
//               A "skin" around the controller is necessary to
//               avoid numerical precision issues.
//               This is dependant on the scale of the users world,
//               but should be a small, positive non zero value.
//               Default: 0.1 
////////////////////////////////////////////////////////////////////
void PhysxControllerDesc::
set_skin_width(float skinWidth) {

  ptr()->skinWidth = skinWidth;
}

////////////////////////////////////////////////////////////////////
//     Function: PhysxControllerDesc::set_step_offset
//       Access: Published
//  Description: Defines the maximum height of an obstacle which the
//               character can climb. 
//               A small value will mean that the character gets
//               stuck and cannot walk up stairs etc, a value which
//               is too large will mean that the character can climb
//               over unrealistically high obstacles.
//               Default: 0.5
////////////////////////////////////////////////////////////////////
void PhysxControllerDesc::
set_step_offset(float stepOffset) {

  ptr()->stepOffset = stepOffset;
}

////////////////////////////////////////////////////////////////////
//     Function: PhysxControllerDesc::set_interaction_flag
//       Access: Published
//  Description: The interaction flag controls if a character
//               controller collides with other controllers.
//               The default is to collide with other controllers.
////////////////////////////////////////////////////////////////////
void PhysxControllerDesc::
set_interaction_flag(bool interactionFlag) {

  ptr()->interactionFlag = (NxCCTInteractionFlag)interactionFlag;
}

////////////////////////////////////////////////////////////////////
//     Function: PhysxControllerDesc::get_pos
//       Access: Published
//  Description: Returns the position of the character.
////////////////////////////////////////////////////////////////////
LPoint3f PhysxControllerDesc::
get_pos() const {

  return PhysxManager::nxExtVec3_to_point3(ptr()->position);
}

////////////////////////////////////////////////////////////////////
//     Function: PhysxControllerDesc::get_slope_limit
//       Access: Published
//  Description: Returns the maximum slope which the character can
//               walk up.
////////////////////////////////////////////////////////////////////
float PhysxControllerDesc::
get_slope_limit() const {

  return NxMath::radToDeg(acosf(ptr()->slopeLimit));
}

////////////////////////////////////////////////////////////////////
//     Function: PhysxControllerDesc::get_skin_width
//       Access: Published
//  Description: Returns the skin width used by the controller. 
////////////////////////////////////////////////////////////////////
float PhysxControllerDesc::
get_skin_width() const {

  return ptr()->skinWidth;
}

////////////////////////////////////////////////////////////////////
//     Function: PhysxControllerDesc::get_step_offset
//       Access: Published
//  Description: Returns the maximum height of an obstacle which the
//               character can climb. 
////////////////////////////////////////////////////////////////////
float PhysxControllerDesc::
get_step_offset() const {

  return ptr()->stepOffset;
}

////////////////////////////////////////////////////////////////////
//     Function: PhysxControllerDesc::get_interaction_flag
//       Access: Published
//  Description: Returns the interaction flag.
////////////////////////////////////////////////////////////////////
bool PhysxControllerDesc::
get_interaction_flag() const {

  return (ptr()->interactionFlag) ? true : false;
}

