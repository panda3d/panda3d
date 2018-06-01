/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file odeAMotorJoint.cxx
 * @author joswilso
 * @date 2006-12-27
 */

#include "config_ode.h"
#include "odeAMotorJoint.h"

TypeHandle OdeAMotorJoint::_type_handle;

OdeAMotorJoint::
OdeAMotorJoint(dJointID id) :
  OdeJoint(id) {
}

OdeAMotorJoint::
OdeAMotorJoint(OdeWorld &world) :
  OdeJoint(dJointCreateAMotor(world.get_id(), nullptr)) {
}

OdeAMotorJoint::
OdeAMotorJoint(OdeWorld &world, OdeJointGroup &joint_group) :
  OdeJoint(dJointCreateAMotor(world.get_id(), joint_group.get_id())) {
}

OdeAMotorJoint::
~OdeAMotorJoint() {
}
