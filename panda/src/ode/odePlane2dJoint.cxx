/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file odePlane2dJoint.cxx
 * @author joswilso
 * @date 2006-12-27
 */

#include "config_ode.h"
#include "odePlane2dJoint.h"

TypeHandle OdePlane2dJoint::_type_handle;

OdePlane2dJoint::
OdePlane2dJoint(dJointID id) :
  OdeJoint(id) {
}

OdePlane2dJoint::
OdePlane2dJoint(OdeWorld &world) :
  OdeJoint(dJointCreatePlane2D(world.get_id(), nullptr)) {
}

OdePlane2dJoint::
OdePlane2dJoint(OdeWorld &world, OdeJointGroup &joint_group) :
  OdeJoint(dJointCreatePlane2D(world.get_id(), joint_group.get_id())) {
}

OdePlane2dJoint::
~OdePlane2dJoint() {
}
