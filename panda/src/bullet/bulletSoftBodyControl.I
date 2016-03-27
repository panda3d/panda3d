/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file bulletSoftBodyControl.I
 * @author enn0x
 * @date 2010-03-04
 */

/**
 *
 */
INLINE void BulletSoftBodyControl::
set_goal(PN_stdfloat goal) {

  _goal = (btScalar)goal;
}

/**
 *
 */
INLINE void BulletSoftBodyControl::
set_max_torque(PN_stdfloat maxtorque) {

  _maxtorque = (btScalar)maxtorque;
}

/**
 *
 */
INLINE void BulletSoftBodyControl::
set_angle(PN_stdfloat angle) {

  _angle = (btScalar)angle;
}

/**
 *
 */
INLINE void BulletSoftBodyControl::
set_sign(PN_stdfloat sign) {

  _sign = (btScalar)sign;
}
