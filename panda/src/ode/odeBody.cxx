/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file odeBody.cxx
 * @author joswilso
 * @date 2006-12-27
 */

#include "config_ode.h"
#include "odeBody.h"
#include "odeJoint.h"

#ifdef HAVE_PYTHON
#include "py_panda.h"
#endif

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

#ifdef HAVE_PYTHON
void OdeBody::
set_data(PyObject *data) {
  Py_XDECREF((PyObject*) dBodyGetData(_id));
  Py_XINCREF(data);
  dBodySetData(_id, data);
}

PyObject* OdeBody::
get_data() const {
  PyObject* data = (PyObject*) dBodyGetData(_id);
  Py_XINCREF(data);
  return data;
}
#endif

OdeJoint OdeBody::
get_joint(int index) const {
  nassertr(_id != nullptr, OdeJoint(nullptr));
  nassertr(index < get_num_joints(), OdeJoint(nullptr));
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
  return (_id != nullptr);
}
