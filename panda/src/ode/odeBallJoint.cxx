/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file odeBallJoint.cxx
 * @author joswilso
 * @date 2006-12-27
 */

#include "config_ode.h"
#include "odeBallJoint.h"

TypeHandle OdeBallJoint::_type_handle;

OdeBallJoint::
OdeBallJoint(dJointID id) :
  OdeJoint(id) {
}

OdeBallJoint::
OdeBallJoint(OdeWorld &world) :
  OdeJoint(dJointCreateBall(world.get_id(), nullptr)) {
}

OdeBallJoint::
OdeBallJoint(OdeWorld &world, OdeJointGroup &joint_group) :
  OdeJoint(dJointCreateBall(world.get_id(), joint_group.get_id())) {
}

OdeBallJoint::
~OdeBallJoint() {
}
