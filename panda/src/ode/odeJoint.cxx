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

////////////////////////////////////////////////////////////////////
//     Function: OdeJoint::attach_bodies
//       Access: Published
//  Description: Attaches two OdeBody objects to this joint.
//               Order is important.
//               Consider using the OdeJoint::attach extension
//               function if you're using the Python interface.
////////////////////////////////////////////////////////////////////
void OdeJoint::
attach_bodies(const OdeBody &body1, const OdeBody &body2) {
  nassertv(body1.get_id() != 0 && body2.get_id() != 0);
  dJointAttach(_id, body1.get_id(), body2.get_id());
}

////////////////////////////////////////////////////////////////////
//     Function: OdeJoint::attach_body
//       Access: Published
//  Description: Attaches a single OdeBody to this joint at the
//               specified index.  The other index will be set to the
//               environment (null).
//               Consider using the OdeJoint::attach extension
//               function if you're using the Python interface.
////////////////////////////////////////////////////////////////////
void OdeJoint::
attach_body(const OdeBody &body, int index) {
  nassertv(body.get_id() != 0);
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

OdeBody OdeJoint::
get_body(int index) const {
  nassertr(index == 0 || index == 1, OdeBody(0));
  return OdeBody(dJointGetBody(_id, index));
}

void OdeJoint::
write(ostream &out, unsigned int indent) const {
  #ifndef NDEBUG //[
  out.width(indent); out << "" << get_type() \
			 << "(id = " << _id \
			 << ", body1 = ";
  OdeBody body = get_body(0);
  if (body.get_id() != 0) {
    body.write(out);
  }
  else {
    out << "0";
  }
  out << ", body2 = ";
  body = get_body(1);
  if (body.get_id() != 0) {
    body.write(out);
  }
  else {
    out << "0";
  }
  out << ")";

  #endif //] NDEBUG
}

bool OdeJoint::
operator==(const OdeJoint &other) {
  return _id == other._id;
}
