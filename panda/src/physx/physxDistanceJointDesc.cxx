/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file physxDistanceJointDesc.cxx
 * @author enn0x
 * @date 2009-09-28
 */

#include "physxDistanceJointDesc.h"
#include "physxSpringDesc.h"

/**
 * Sets the maximum rest length of the rope or rod between the two anchor
 * points.
 */
void PhysxDistanceJointDesc::
set_max_distance(float distance) {

  _desc.maxDistance = distance;
}

/**
 * Sets the minimum rest length of the rope or rod between the two anchor points
 */
void PhysxDistanceJointDesc::
set_min_distance(float distance) {

  _desc.minDistance = distance;
}

/**
 * Makes the joint springy.  The spring.targetValue is not used.
 */
void PhysxDistanceJointDesc::
set_spring(const PhysxSpringDesc &spring) {

  _desc.spring = spring._desc;
}

/**
 * Sets or clears a single DistanceJointFlag flag.
 */
void PhysxDistanceJointDesc::
set_flag(PhysxDistanceJointFlag flag, bool value) {

  if (value == true) {
    _desc.flags |= flag;
  }
  else {
    _desc.flags &= ~(flag);
  }
}

/**

 */
float PhysxDistanceJointDesc::
get_max_distance() const {

  return _desc.maxDistance;
}

/**

 */
float PhysxDistanceJointDesc::
get_min_distance() const {

  return _desc.minDistance;
}

/**

 */
PhysxSpringDesc PhysxDistanceJointDesc::
get_spring() const {

  PhysxSpringDesc value;
  value._desc = _desc.spring;
  return value;
}

/**
 * Return a single DistanceJointFlag flag.
 */
bool PhysxDistanceJointDesc::
get_flag(PhysxDistanceJointFlag flag) const {

  return (_desc.flags & flag) ? true : false;
}
