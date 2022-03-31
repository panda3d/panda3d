/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file odeContactJoint.cxx
 * @author joswilso
 * @date 2006-12-27
 */

#include "config_ode.h"
#include "odeContactJoint.h"

TypeHandle OdeContactJoint::_type_handle;

OdeContactJoint::
OdeContactJoint(dJointID id) :
  OdeJoint(id) {
}

OdeContactJoint::
OdeContactJoint(OdeWorld &world, const OdeContact &contact) :
  OdeJoint(dJointCreateContact(world.get_id(), nullptr, contact.get_contact_ptr())) {
}

OdeContactJoint::
OdeContactJoint(OdeWorld &world, OdeJointGroup &joint_group, const OdeContact &contact) :
  OdeJoint(dJointCreateContact(world.get_id(), joint_group.get_id(), contact.get_contact_ptr())) {
}

OdeContactJoint::
~OdeContactJoint() {
}
