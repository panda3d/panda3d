// Filename: odePlane2dJoint.I
// Created by:  joswilso (27Dec06)
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

INLINE void OdePlane2dJoint::
set_x_param(int parameter, dReal value) {
  dJointSetPlane2DXParam(_id, parameter, value);
}

INLINE void OdePlane2dJoint::
set_y_param(int parameter, dReal value) {
  dJointSetPlane2DYParam(_id, parameter, value);
}

INLINE void OdePlane2dJoint::
set_angle_param(int parameter, dReal value) {
  dJointSetPlane2DAngleParam(_id, parameter, value);
}
