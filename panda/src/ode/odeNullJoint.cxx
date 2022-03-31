/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file odeNullJoint.cxx
 * @author joswilso
 * @date 2006-12-27
 */

#include "config_ode.h"
#include "odeNullJoint.h"

TypeHandle OdeNullJoint::_type_handle;

OdeNullJoint::
OdeNullJoint(dJointID id) :
  OdeJoint(id) {
}

OdeNullJoint::
OdeNullJoint(OdeWorld &world) :
  OdeJoint(dJointCreateNull(world.get_id(), nullptr)) {
}

OdeNullJoint::
OdeNullJoint(OdeWorld &world, OdeJointGroup &joint_group) :
  OdeJoint(dJointCreateNull(world.get_id(), joint_group.get_id())) {
}

OdeNullJoint::
~OdeNullJoint() {
}
