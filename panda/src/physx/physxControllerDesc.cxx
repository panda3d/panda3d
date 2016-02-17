/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file physxControllerDesc.cxx
 * @author enn0x
 * @date 2009-09-22
 */

#include "physxControllerDesc.h"
#include "physxManager.h"

/**
 * Set the position of the character.
 */
void PhysxControllerDesc::
set_pos(const LPoint3f &pos) {

  ptr()->position = PhysxManager::point3_to_nxExtVec3(pos);
}

/**
 * Sets the maximum slope which the character can walk up.  In general it is
 * desirable to limit where the character can walk, in particular it is
 * unrealistic for the character to be able to climb arbitary slopes.  The value
 * is expressed in degrees.  Default: 45.0 degrees.
 */
void PhysxControllerDesc::
set_slope_limit(float slopeLimit) {

  ptr()->slopeLimit = cosf(NxMath::degToRad(slopeLimit));
}

/**
 * Sets the skin width used by the controller.  A "skin" around the controller
 * is necessary to avoid numerical precision issues.  This is dependant on the
 * scale of the users world, but should be a small, positive non zero value.
 * Default: 0.1
 */
void PhysxControllerDesc::
set_skin_width(float skinWidth) {

  ptr()->skinWidth = skinWidth;
}

/**
 * Defines the maximum height of an obstacle which the character can climb.  A
 * small value will mean that the character gets stuck and cannot walk up stairs
 * etc, a value which is too large will mean that the character can climb over
 * unrealistically high obstacles.  Default: 0.5
 */
void PhysxControllerDesc::
set_step_offset(float stepOffset) {

  ptr()->stepOffset = stepOffset;
}

/**
 * The interaction flag controls if a character controller collides with other
 * controllers.  The default is to collide with other controllers.
 */
void PhysxControllerDesc::
set_interaction_flag(bool interactionFlag) {

  ptr()->interactionFlag = (NxCCTInteractionFlag)interactionFlag;
}

/**
 * Returns the position of the character.
 */
LPoint3f PhysxControllerDesc::
get_pos() const {

  return PhysxManager::nxExtVec3_to_point3(ptr()->position);
}

/**
 * Returns the maximum slope which the character can walk up.
 */
float PhysxControllerDesc::
get_slope_limit() const {

  return NxMath::radToDeg(acosf(ptr()->slopeLimit));
}

/**
 * Returns the skin width used by the controller.
 */
float PhysxControllerDesc::
get_skin_width() const {

  return ptr()->skinWidth;
}

/**
 * Returns the maximum height of an obstacle which the character can climb.
 */
float PhysxControllerDesc::
get_step_offset() const {

  return ptr()->stepOffset;
}

/**
 * Returns the interaction flag.
 */
bool PhysxControllerDesc::
get_interaction_flag() const {

  return (ptr()->interactionFlag) ? true : false;
}
