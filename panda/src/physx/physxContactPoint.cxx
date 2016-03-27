/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file physxContactPoint.cxx
 * @author enn0x
 * @date 2009-12-20
 */

#include "physxContactPoint.h"
#include "physxManager.h"
#include "physxShape.h"

TypeHandle PhysxContactPoint::_type_handle;

/**
 *
 */
void PhysxContactPoint::
set(NxContactStreamIterator it) {

  _point = it.getPoint();
  _normal = it.getPatchNormal();
  _normal_force = it.getPointNormalForce();
  _separation = it.getSeparation();
  _feature_index0 = it.getFeatureIndex0();
  _feature_index1 = it.getFeatureIndex1();
}

/**
 *
 */
PhysxContactPoint PhysxContactPoint::
empty() {

  return PhysxContactPoint();
}


/**
 * Returns the contact point position.
 */
LPoint3f PhysxContactPoint::
get_point() const {

  return PhysxManager::nxVec3_to_point3(_point);
}

/**
 * Retrieves the patch normal.
 */
LVector3f PhysxContactPoint::
get_normal() const {

  return PhysxManager::nxVec3_to_vec3(_normal);
}

/**
 * Retrieves the point normal force.
 */
float PhysxContactPoint::
get_normal_force() const {

  return _normal_force;
}

/**
 * Return the separation for the contact point.
 */
float PhysxContactPoint::
get_separation() const {

  return _separation;
}

/**
 * Retrieves the feature index.
 */
unsigned int PhysxContactPoint::
get_feature_index0() const {

  return _feature_index0;
}

/**
 * Retrieves the feature index.
 */
unsigned int PhysxContactPoint::
get_feature_index1() const {

  return _feature_index1;
}
