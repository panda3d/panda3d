// Filename: odeNullJoint.cxx
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
#include "odeNullJoint.h"

TypeHandle OdeNullJoint::_type_handle;

OdeNullJoint::
OdeNullJoint(dJointID id) : 
  OdeJoint(id) {
}

OdeNullJoint::
OdeNullJoint(OdeWorld &world) : 
  OdeJoint(dJointCreateNull(world.get_id(), 0)) {
}

OdeNullJoint::
OdeNullJoint(OdeWorld &world, OdeJointGroup &joint_group) : 
  OdeJoint(dJointCreateNull(world.get_id(), joint_group.get_id())) {
}

OdeNullJoint::
~OdeNullJoint() {
}
