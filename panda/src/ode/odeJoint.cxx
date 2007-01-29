// Filename: odeJoint.cxx
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
#include "odeJoint.h"

TypeHandle OdeJoint::_type_handle;

OdeJoint::
OdeJoint(dJointID id) : 
  _id(id) {
  ostream &out = odejoint_cat.debug();
  out << get_type() << "(" << _id  << ")\n";
}

OdeJoint::
~OdeJoint() {
}

void OdeJoint::
destroy() {
  dJointDestroy(_id);
}

void OdeJoint::
attach(const OdeBody &body1, const OdeBody &body2) {
  dJointAttach(_id, body1.get_id(), body2.get_id());
}

void OdeJoint::
attach(const OdeBody &body, int index) {
  if (index == 0) {
    dJointAttach(_id, body.get_id(), 0);
  } else {
    dJointAttach(_id, 0, body.get_id());
  }
}

void OdeJoint::
detach() {
  dJointAttach(_id, 0, 0);
}

void OdeJoint::
get_body(int index, OdeBody &body) const {
  body._id = dJointGetBody(_id, index);
}

void OdeJoint::
write(ostream &out, unsigned int indent) const {
  #ifndef NDEBUG //[
  out.width(indent); out << "" << get_type() \
			 << "(id = " << _id \
			 << ", body1 = ";
  OdeBody body;
  get_body(0, body);
  if (body.get_id() != 0) {
    body.write(out);
  }
  else {
    out << "0";
  }
  out << ", body2 = ";
  get_body(1, body);
  if (body.get_id() != 0) {
    body.write(out);
  }
  else {
    out << "0";
  }
  out << ")";

  #endif //] NDEBUG
}
