/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file odeJoint_ext.cxx
 * @author rdb
 * @date 2013-12-11
 */

#include "odeJoint_ext.h"

#ifdef HAVE_PYTHON

#include "odeJoint.h"
#include "odeBallJoint.h"
#include "odeHingeJoint.h"
#include "odeSliderJoint.h"
#include "odeContactJoint.h"
#include "odeUniversalJoint.h"
#include "odeHinge2Joint.h"
#include "odeFixedJoint.h"
#include "odeNullJoint.h"
#include "odeAMotorJoint.h"
#include "odeLMotorJoint.h"
#include "odePlane2dJoint.h"

#ifndef CPPPARSER
extern Dtool_PyTypedObject Dtool_OdeBody;
extern Dtool_PyTypedObject Dtool_OdeJoint;
extern Dtool_PyTypedObject Dtool_OdeBallJoint;
extern Dtool_PyTypedObject Dtool_OdeHingeJoint;
extern Dtool_PyTypedObject Dtool_OdeSliderJoint;
extern Dtool_PyTypedObject Dtool_OdeContactJoint;
extern Dtool_PyTypedObject Dtool_OdeUniversalJoint;
extern Dtool_PyTypedObject Dtool_OdeHinge2Joint;
extern Dtool_PyTypedObject Dtool_OdeFixedJoint;
extern Dtool_PyTypedObject Dtool_OdeNullJoint;
extern Dtool_PyTypedObject Dtool_OdeAMotorJoint;
extern Dtool_PyTypedObject Dtool_OdeLMotorJoint;
extern Dtool_PyTypedObject Dtool_OdePlane2dJoint;
#endif

/**
 * Attach two bodies together.  If either body is None, the other will be
 * attached to the environment.
 */
void Extension<OdeJoint>::
attach(PyObject *param1, PyObject *param2) {
  const OdeBody *body1 = nullptr;
  if (param1 != Py_None) {
    body1 = (const OdeBody *)DTOOL_Call_GetPointerThisClass(param1, &Dtool_OdeBody, 1, "OdeJoint.attach", true, true);
    if (body1 == nullptr) {
      return;
    }
  }

  const OdeBody *body2 = nullptr;
  if (param2 != Py_None) {
    body2 = (const OdeBody *)DTOOL_Call_GetPointerThisClass(param2, &Dtool_OdeBody, 2, "OdeJoint.attach", true, true);
    if (body2 == nullptr) {
      return;
    }
  }

  if (body1 && body2) {
    _this->attach_bodies(*body1, *body2);

  } else if (body1 && !body2) {
    _this->attach_body(*body1, 0);

  } else if (!body1 && body2) {
    _this->attach_body(*body2, 1);
  }
}

/**
 * Do a sort of pseudo-downcast on this space in order to expose its
 * specialized functions.
 */
PyObject *Extension<OdeJoint>::
convert() const {
  Dtool_PyTypedObject *class_type;
  OdeJoint *joint;

  switch (_this->get_joint_type()) {
  case OdeJoint::JT_ball:
    joint = new OdeBallJoint(_this->get_id());
    class_type = &Dtool_OdeBallJoint;
    break;

  case OdeJoint::JT_hinge:
    joint = new OdeHingeJoint(_this->get_id());
    class_type = &Dtool_OdeHingeJoint;
    break;

  case OdeJoint::JT_slider:
    joint = new OdeSliderJoint(_this->get_id());
    class_type = &Dtool_OdeSliderJoint;
    break;

  case OdeJoint::JT_contact:
    joint = new OdeContactJoint(_this->get_id());
    class_type = &Dtool_OdeContactJoint;
    break;

  case OdeJoint::JT_universal:
    joint = new OdeUniversalJoint(_this->get_id());
    class_type = &Dtool_OdeUniversalJoint;
    break;

  case OdeJoint::JT_hinge2:
    joint = new OdeHinge2Joint(_this->get_id());
    class_type = &Dtool_OdeHinge2Joint;
    break;

  case OdeJoint::JT_fixed:
    joint = new OdeFixedJoint(_this->get_id());
    class_type = &Dtool_OdeFixedJoint;
    break;

  case OdeJoint::JT_null:
    joint = new OdeNullJoint(_this->get_id());
    class_type = &Dtool_OdeNullJoint;
    break;

  case OdeJoint::JT_a_motor:
    joint = new OdeAMotorJoint(_this->get_id());
    class_type = &Dtool_OdeAMotorJoint;
    break;

  case OdeJoint::JT_l_motor:
    joint = new OdeLMotorJoint(_this->get_id());
    class_type = &Dtool_OdeLMotorJoint;
    break;

  case OdeJoint::JT_plane2d:
    joint = new OdePlane2dJoint(_this->get_id());
    class_type = &Dtool_OdePlane2dJoint;
    break;

  default:
    // This shouldn't happen, but if it does, we should just return a regular
    // OdeJoint.
    joint = new OdeJoint(_this->get_id());
    class_type = &Dtool_OdeJoint;
  }

  return DTool_CreatePyInstanceTyped((void *)joint, *class_type,
                                     true, false, joint->get_type_index());
}

#endif  // HAVE_PYTHON
