// Filename: odeFixedJoint.cxx
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
#include "odeFixedJoint.h"

TypeHandle OdeFixedJoint::_type_handle;

OdeFixedJoint::
OdeFixedJoint(dJointID id) : 
  OdeJoint(id) {
}

OdeFixedJoint::
OdeFixedJoint(OdeWorld &world) : 
  OdeJoint(dJointCreateFixed(world.get_id(), 0)) {
}

OdeFixedJoint::
OdeFixedJoint(OdeWorld &world, OdeJointGroup &joint_group) : 
  OdeJoint(dJointCreateFixed(world.get_id(), joint_group.get_id())) {
}

OdeFixedJoint::
~OdeFixedJoint() {
}
