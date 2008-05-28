// Filename: odeHingeJoint.cxx
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
#include "odeHingeJoint.h"

TypeHandle OdeHingeJoint::_type_handle;

OdeHingeJoint::
OdeHingeJoint(dJointID id) : 
  OdeJoint(id) {
}

OdeHingeJoint::
OdeHingeJoint(OdeWorld &world) : 
  OdeJoint(dJointCreateHinge(world.get_id(), 0)) {
}

OdeHingeJoint::
OdeHingeJoint(OdeWorld &world, OdeJointGroup &joint_group) : 
  OdeJoint(dJointCreateHinge(world.get_id(), joint_group.get_id())) {
}

OdeHingeJoint::
~OdeHingeJoint() {
}
