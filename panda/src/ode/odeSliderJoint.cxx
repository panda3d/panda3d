// Filename: odeSliderJoint.cxx
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
#include "odeSliderJoint.h"

TypeHandle OdeSliderJoint::_type_handle;

OdeSliderJoint::
OdeSliderJoint(dJointID id) : 
  OdeJoint(id) {
}

OdeSliderJoint::
OdeSliderJoint(OdeWorld &world) : 
  OdeJoint(dJointCreateSlider(world.get_id(), 0)) {
}

OdeSliderJoint::
OdeSliderJoint(OdeWorld &world, OdeJointGroup &joint_group) : 
  OdeJoint(dJointCreateSlider(world.get_id(), joint_group.get_id())) {
}

OdeSliderJoint::
~OdeSliderJoint() {
}
