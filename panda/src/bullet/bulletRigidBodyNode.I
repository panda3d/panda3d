/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file bulletRigidBodyNode.I
 * @author enn0x
 * @date 2010-11-19
 */

/**
 *
 */
INLINE BulletRigidBodyNode::
~BulletRigidBodyNode() {

  delete _rigid;
}

/**
 *
 */
INLINE void BulletRigidBodyNode::
set_linear_damping(PN_stdfloat value) {

  _rigid->setDamping(value, _rigid->getAngularDamping());
}

/**
 *
 */
INLINE void BulletRigidBodyNode::
set_angular_damping(PN_stdfloat value) {

  _rigid->setDamping(_rigid->getLinearDamping(), value);
}

/**
 *
 */
INLINE PN_stdfloat BulletRigidBodyNode::
get_linear_damping() const {

  return (PN_stdfloat)_rigid->getLinearDamping();
}

/**
 *
 */
INLINE PN_stdfloat BulletRigidBodyNode::
get_angular_damping() const {

  return (PN_stdfloat)_rigid->getAngularDamping();
}
