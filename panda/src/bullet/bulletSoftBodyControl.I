// Filename: bulletSoftBodyControl.I
// Created by:  enn0x (04Mar10)
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


////////////////////////////////////////////////////////////////////
//     Function: BulletSoftBodyControl::set_goal
//       Access: Published
//  Description:
////////////////////////////////////////////////////////////////////
INLINE void BulletSoftBodyControl::
set_goal(PN_stdfloat goal) {

  _goal = (btScalar)goal;
}

////////////////////////////////////////////////////////////////////
//     Function: BulletSoftBodyControl::set_max_torque
//       Access: Published
//  Description:
////////////////////////////////////////////////////////////////////
INLINE void BulletSoftBodyControl::
set_max_torque(PN_stdfloat maxtorque) {

  _maxtorque = (btScalar)maxtorque;
}

////////////////////////////////////////////////////////////////////
//     Function: BulletSoftBodyControl::set_angle
//       Access: Published
//  Description:
////////////////////////////////////////////////////////////////////
INLINE void BulletSoftBodyControl::
set_angle(PN_stdfloat angle) {

  _angle = (btScalar)angle;
}

////////////////////////////////////////////////////////////////////
//     Function: BulletSoftBodyControl::set_sign
//       Access: Published
//  Description:
////////////////////////////////////////////////////////////////////
INLINE void BulletSoftBodyControl::
set_sign(PN_stdfloat sign) {

  _sign = (btScalar)sign;
}

