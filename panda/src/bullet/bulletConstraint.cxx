/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file bulletConstraint.cxx
 * @author enn0x
 * @date 2010-03-01
 */

#include "bulletConstraint.h"

#include "bulletRigidBodyNode.h"
#include "bulletShape.h"

TypeHandle BulletConstraint::_type_handle;

/**
 *
 */
void BulletConstraint::
enable_feedback(bool value) {

  ptr()->enableFeedback(value);
}

/**
 *
 */
PN_stdfloat BulletConstraint::
get_applied_impulse() const {

  return (PN_stdfloat)ptr()->getAppliedImpulse();
}

/**
 *
 */
void BulletConstraint::
set_debug_draw_size(PN_stdfloat size) {

  ptr()->setDbgDrawSize((btScalar)size);
}

/**
 *
 */
PN_stdfloat BulletConstraint::
get_debug_draw_size() {

  return (PN_stdfloat)ptr()->getDbgDrawSize();
}

/**
 *
 */
BulletRigidBodyNode *BulletConstraint::
get_rigid_body_a() {

  return (BulletRigidBodyNode *)ptr()->getRigidBodyA().getUserPointer();
}

/**
 *
 */
BulletRigidBodyNode *BulletConstraint::
get_rigid_body_b() {

  return (BulletRigidBodyNode *)ptr()->getRigidBodyB().getUserPointer();
}

/**
 *
 */
void BulletConstraint::
set_param(ConstraintParam num, PN_stdfloat value, int axis) {

  ptr()->setParam((btConstraintParams)num, (btScalar)value, axis);
}

/**
 *
 */
PN_stdfloat BulletConstraint::
get_param(ConstraintParam num, int axis) {

  return (PN_stdfloat)ptr()->getParam((btConstraintParams)num, axis);
}
