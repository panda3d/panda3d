/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file odeUniversalJoint.cxx
 * @author joswilso
 * @date 2006-12-27
 */

#include "config_ode.h"
#include "odeUniversalJoint.h"

TypeHandle OdeUniversalJoint::_type_handle;

OdeUniversalJoint::
OdeUniversalJoint(dJointID id) :
  OdeJoint(id) {
}

OdeUniversalJoint::
OdeUniversalJoint(OdeWorld &world) :
  OdeJoint(dJointCreateUniversal(world.get_id(), nullptr)) {
}

OdeUniversalJoint::
OdeUniversalJoint(OdeWorld &world,  OdeJointGroup &joint_group) :
  OdeJoint(dJointCreateUniversal(world.get_id(), joint_group.get_id())) {
}

OdeUniversalJoint::
~OdeUniversalJoint() {
}
