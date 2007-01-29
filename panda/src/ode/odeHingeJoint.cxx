// Filename: odeHingeJoint.cxx
// Created by:  joswilso (27Dec06)
//
////////////////////////////////////////////////////////////////////
//
// PANDA 3D SOFTWARE
// Copyright (c) 2001 - 2004, Disney Enterprises, Inc.  All rights reserved
//
// All use of this software is subject to the terms of the Panda 3d
// Software license.  You should have received a copy of this license
// along with this source code; you will also find a current copy of
// the license at http://etc.cmu.edu/panda3d/docs/license/ .
//
// To contact the maintainers of this program write to
// panda3d-general@lists.sourceforge.net .
//
////////////////////////////////////////////////////////////////////

#include "config_ode.h"
#include "odeHingeJoint.h"

TypeHandle OdeHingeJoint::_type_handle;

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
