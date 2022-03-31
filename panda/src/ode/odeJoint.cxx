/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file odeJoint.cxx
 * @author joswilso
 * @date 2006-12-27
 */

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
  _id(nullptr) {
  if (odejoint_cat.is_debug()) {
    std::ostream &out = odejoint_cat.debug();
    out << get_type() << "(" << _id  << ")\n";
  }
}

OdeJoint::
OdeJoint(dJointID id) :
  _id(id) {
  if (odejoint_cat.is_debug()) {
    std::ostream &out = odejoint_cat.debug();
    out << get_type() << "(" << _id  << ")\n";
  }
}

OdeJoint::
~OdeJoint() {
}

void OdeJoint::
destroy() {
  nassertv(_id);
  dJointDestroy(_id);
}

/**
 * Attaches two OdeBody objects to this joint.  Order is important.  Consider
 * using the OdeJoint::attach extension function if you're using the Python
 * interface.
 */
void OdeJoint::
attach_bodies(const OdeBody &body1, const OdeBody &body2) {
  nassertv(_id);
  nassertv(body1.get_id() != nullptr || body2.get_id() != nullptr);
  dJointAttach(_id, body1.get_id(), body2.get_id());
}

/**
 * Attaches a single OdeBody to this joint at the specified index (0 or 1).
 * The other index will be set to the environment (null). Consider using the
 * OdeJoint::attach extension function if you're using the Python interface.
 */
void OdeJoint::
attach_body(const OdeBody &body, int index) {
  nassertv(_id);
  nassertv(body.get_id() != nullptr);
  nassertv(index == 0 || index == 1);
  if (index == 0) {
    dJointAttach(_id, body.get_id(), nullptr);
  } else {
    dJointAttach(_id, nullptr, body.get_id());
  }
}

void OdeJoint::
detach() {
  nassertv(_id);
  dJointAttach(_id, nullptr, nullptr);
}

OdeBody OdeJoint::
get_body(int index) const {
  nassertr(_id, OdeBody(nullptr));
  nassertr(index == 0 || index == 1, OdeBody(nullptr));
  return OdeBody(dJointGetBody(_id, index));
}

void OdeJoint::
write(std::ostream &out, unsigned int indent) const {
  out.width(indent); out << "" << get_type() \
                         << "(id = " << _id \
                         << ", body1 = ";
  OdeBody body = get_body(0);
  if (body.get_id() != nullptr) {
    body.write(out);
  }
  else {
    out << "0";
  }
  out << ", body2 = ";
  body = get_body(1);
  if (body.get_id() != nullptr) {
    body.write(out);
  }
  else {
    out << "0";
  }
  out << ")";

}

OdeJoint::
operator bool () const {
  return (_id != nullptr);
}

OdeBallJoint OdeJoint::
convert_to_ball() const {
  nassertr(_id != nullptr, OdeBallJoint(nullptr));
  nassertr(get_joint_type() == JT_ball, OdeBallJoint(nullptr));
  return OdeBallJoint(_id);
}

OdeHingeJoint OdeJoint::
convert_to_hinge() const {
  nassertr(_id != nullptr, OdeHingeJoint(nullptr));
  nassertr(get_joint_type() == JT_hinge, OdeHingeJoint(nullptr));
  return OdeHingeJoint(_id);
}

OdeSliderJoint OdeJoint::
convert_to_slider() const {
  nassertr(_id != nullptr, OdeSliderJoint(nullptr));
  nassertr(get_joint_type() == JT_slider, OdeSliderJoint(nullptr));
  return OdeSliderJoint(_id);
}

OdeContactJoint OdeJoint::
convert_to_contact() const {
  nassertr(_id != nullptr, OdeContactJoint(nullptr));
  nassertr(get_joint_type() == JT_contact, OdeContactJoint(nullptr));
  return OdeContactJoint(_id);
}

OdeUniversalJoint OdeJoint::
convert_to_universal() const {
  nassertr(_id != nullptr, OdeUniversalJoint(nullptr));
  nassertr(get_joint_type() == JT_universal, OdeUniversalJoint(nullptr));
  return OdeUniversalJoint(_id);
}

OdeHinge2Joint OdeJoint::
convert_to_hinge2() const {
  nassertr(_id != nullptr, OdeHinge2Joint(nullptr));
  nassertr(get_joint_type() == JT_hinge2, OdeHinge2Joint(nullptr));
  return OdeHinge2Joint(_id);
}

OdeFixedJoint OdeJoint::
convert_to_fixed() const {
  nassertr(_id != nullptr, OdeFixedJoint(nullptr));
  nassertr(get_joint_type() == JT_fixed, OdeFixedJoint(nullptr));
  return OdeFixedJoint(_id);
}

OdeNullJoint OdeJoint::
convert_to_null() const {
  nassertr(_id != nullptr, OdeNullJoint(nullptr));
  nassertr(get_joint_type() == JT_null, OdeNullJoint(nullptr));
  return OdeNullJoint(_id);
}

OdeAMotorJoint OdeJoint::
convert_to_a_motor() const {
  nassertr(_id != nullptr, OdeAMotorJoint(nullptr));
  nassertr(get_joint_type() == JT_a_motor, OdeAMotorJoint(nullptr));
  return OdeAMotorJoint(_id);
}

OdeLMotorJoint OdeJoint::
convert_to_l_motor() const {
  nassertr(_id != nullptr, OdeLMotorJoint(nullptr));
  nassertr(get_joint_type() == JT_l_motor, OdeLMotorJoint(nullptr));
  return OdeLMotorJoint(_id);
}

OdePlane2dJoint OdeJoint::
convert_to_plane2d() const {
  nassertr(_id != nullptr, OdePlane2dJoint(nullptr));
  nassertr(get_joint_type() == JT_plane2d, OdePlane2dJoint(nullptr));
  return OdePlane2dJoint(_id);
}
