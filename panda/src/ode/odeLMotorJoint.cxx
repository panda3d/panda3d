/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file odeLMotorJoint.cxx
 * @author joswilso
 * @date 2006-12-27
 */

#include "config_ode.h"
#include "odeLMotorJoint.h"

TypeHandle OdeLMotorJoint::_type_handle;

OdeLMotorJoint::
OdeLMotorJoint(dJointID id) :
  OdeJoint(id) {
}

OdeLMotorJoint::
OdeLMotorJoint(OdeWorld &world  ) :
  OdeJoint(dJointCreateLMotor(world.get_id(), nullptr)) {
}

OdeLMotorJoint::
OdeLMotorJoint(OdeWorld &world, OdeJointGroup &joint_group) :
  OdeJoint(dJointCreateLMotor(world.get_id(), joint_group.get_id())) {
}

OdeLMotorJoint::
~OdeLMotorJoint() {
}
