// Filename: odeJoint.cxx
// Created by:  joswilso (27Dec06)
//
////////////////////////////////////////////////////////////////////
//
// PANDA 3D SOFTWARE
// Copyright (c) Carnegie Mellon University.  All rights reserved.
//
// All use of this software is subject to the terms of the revised BSD
// license.  You should have received a copy of this license along
// with this source code in a file named "LICENSE."
//
////////////////////////////////////////////////////////////////////

#include "config_ode.h"
#include "odeJoint.h"
#include "odeBallJoint.h"
#include "odeHingeJoint.h"
#include "odeHinge2Joint.h"
#include "odeSliderJoint.h"
#include "odeContactJoint.h"
#include "odeUniversalJoint.h"
#include "odeFixedJoint.h"
#include "odeNullJoint.h"
#include "odePlane2dJoint.h"
#include "odeAMotorJoint.h"
#include "odeLMotorJoint.h"
#include "odeBody.h"

TypeHandle OdeJoint::_type_handle;

OdeJoint::
OdeJoint() : 
  _id(0) {
  ostream &out = odejoint_cat.debug();
  out << get_type() << "(" << _id  << ")\n";
}

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
  nassertv(_id);
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
  nassertv(_id);
  nassertv(body1.get_id() != 0 || body2.get_id() != 0);
  dJointAttach(_id, body1.get_id(), body2.get_id());
}

////////////////////////////////////////////////////////////////////
//     Function: OdeJoint::attach_body
//       Access: Published
//  Description: Attaches a single OdeBody to this joint at the
//               specified index (0 or 1).  The other index will be 
//               set to the environment (null).
//               Consider using the OdeJoint::attach extension
//               function if you're using the Python interface.
////////////////////////////////////////////////////////////////////
void OdeJoint::
attach_body(const OdeBody &body, int index) {
  nassertv(_id);
  nassertv(body.get_id() != 0);
  nassertv(index == 0 || index == 1);
  if (index == 0) {
    dJointAttach(_id, body.get_id(), 0);
  } else {
    dJointAttach(_id, 0, body.get_id());
  }
}

void OdeJoint::
detach() {
  nassertv(_id);
  dJointAttach(_id, 0, 0);
}

OdeBody OdeJoint::
get_body(int index) const {
  nassertr(_id, OdeBody(0));
  nassertr(index == 0 || index == 1, OdeBody(0));
  return OdeBody(dJointGetBody(_id, index));
}

void OdeJoint::
write(ostream &out, unsigned int indent) const {
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

}

OdeJoint::
operator bool () const {
  return (_id != NULL);
}

OdeBallJoint OdeJoint::
convert_to_ball() const {
  nassertr(_id != 0, OdeBallJoint(0));
  nassertr(get_joint_type() == JT_ball, OdeBallJoint(0));
  return OdeBallJoint(_id);
}

OdeHingeJoint OdeJoint::
convert_to_hinge() const {
  nassertr(_id != 0, OdeHingeJoint(0));
  nassertr(get_joint_type() == JT_hinge, OdeHingeJoint(0));
  return OdeHingeJoint(_id);
}

OdeSliderJoint OdeJoint::
convert_to_slider() const {
  nassertr(_id != 0, OdeSliderJoint(0));
  nassertr(get_joint_type() == JT_slider, OdeSliderJoint(0));
  return OdeSliderJoint(_id);
}

OdeContactJoint OdeJoint::
convert_to_contact() const {
  nassertr(_id != 0, OdeContactJoint(0));
  nassertr(get_joint_type() == JT_contact, OdeContactJoint(0));
  return OdeContactJoint(_id);
}

OdeUniversalJoint OdeJoint::
convert_to_universal() const {
  nassertr(_id != 0, OdeUniversalJoint(0));
  nassertr(get_joint_type() == JT_universal, OdeUniversalJoint(0));
  return OdeUniversalJoint(_id);
}

OdeHinge2Joint OdeJoint::
convert_to_hinge2() const {
  nassertr(_id != 0, OdeHinge2Joint(0));
  nassertr(get_joint_type() == JT_hinge2, OdeHinge2Joint(0));
  return OdeHinge2Joint(_id);
}

OdeFixedJoint OdeJoint::
convert_to_fixed() const {
  nassertr(_id != 0, OdeFixedJoint(0));
  nassertr(get_joint_type() == JT_fixed, OdeFixedJoint(0));
  return OdeFixedJoint(_id);
}

OdeNullJoint OdeJoint::
convert_to_null() const {
  nassertr(_id != 0, OdeNullJoint(0));
  nassertr(get_joint_type() == JT_null, OdeNullJoint(0));
  return OdeNullJoint(_id);
}

OdeAMotorJoint OdeJoint::
convert_to_a_motor() const {
  nassertr(_id != 0, OdeAMotorJoint(0));
  nassertr(get_joint_type() == JT_a_motor, OdeAMotorJoint(0));
  return OdeAMotorJoint(_id);
}

OdeLMotorJoint OdeJoint::
convert_to_l_motor() const {
  nassertr(_id != 0, OdeLMotorJoint(0));
  nassertr(get_joint_type() == JT_l_motor, OdeLMotorJoint(0));
  return OdeLMotorJoint(_id);
}

OdePlane2dJoint OdeJoint::
convert_to_plane2d() const {
  nassertr(_id != 0, OdePlane2dJoint(0));
  nassertr(get_joint_type() == JT_plane2d, OdePlane2dJoint(0));
  return OdePlane2dJoint(_id);
}

