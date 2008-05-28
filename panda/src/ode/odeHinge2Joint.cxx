// Filename: odeHinge2Joint.cxx
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
#include "odeHinge2Joint.h"

TypeHandle OdeHinge2Joint::_type_handle;

OdeHinge2Joint::
OdeHinge2Joint(dJointID id) : 
  OdeJoint(id) {
}

OdeHinge2Joint::
OdeHinge2Joint(OdeWorld &world) : 
  OdeJoint(dJointCreateHinge2(world.get_id(), 0)) {
}

OdeHinge2Joint::
OdeHinge2Joint(OdeWorld &world, OdeJointGroup &joint_group) : 
  OdeJoint(dJointCreateHinge2(world.get_id(), joint_group.get_id())) {
}

OdeHinge2Joint::
~OdeHinge2Joint() {
}
