// Filename: odeBallJoint.cxx
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

#include "config_ode.h"
#include "odeBallJoint.h"

TypeHandle OdeBallJoint::_type_handle;

OdeBallJoint::
OdeBallJoint(dJointID id) : 
  OdeJoint(id) {
}

OdeBallJoint::
OdeBallJoint(OdeWorld &world) : 
  OdeJoint(dJointCreateBall(world.get_id(), 0)) {
}

OdeBallJoint::
OdeBallJoint(OdeWorld &world, OdeJointGroup &joint_group) : 
  OdeJoint(dJointCreateBall(world.get_id(), joint_group.get_id())) {
}

OdeBallJoint::
~OdeBallJoint() {
}
