// Filename: odeBody.cxx
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
#include "odeBody.h"
#include "odeJoint.h"

TypeHandle OdeBody::_type_handle;

OdeBody::
OdeBody(dBodyID id) :
  _id(id) {
}

OdeBody::
OdeBody(OdeWorld &world) :
  _id(dBodyCreate(world.get_id())) {
      world.add_body_dampening(*this, 0);
}

OdeBody::
~OdeBody() {
}

void OdeBody::
destroy() {
#ifdef HAVE_PYTHON
  Py_XDECREF((PyObject*) dBodyGetData(_id));
#endif
  nassertv(_id);
  dBodyDestroy(_id);
}

OdeJoint OdeBody::
get_joint(int index) const {
  nassertr(_id != 0, OdeJoint(0));
  nassertr(index < get_num_joints(), OdeJoint(0)); 
  return OdeJoint(dBodyGetJoint(_id, index));
}

void OdeBody::
write(ostream &out, unsigned int indent) const {
  out.width(indent); out << "" << get_type() \
                         << "(id = " << _id \
                         << ")";
}

OdeBody::
operator bool () const {
  return (_id != NULL);
}
