/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file odeSpace_ext.cxx
 * @author rdb
 * @date 2013-12-10
 */

#include "odeSpace_ext.h"
#include "config_ode.h"

#ifdef HAVE_PYTHON

#include "odeGeom.h"
#include "odeHashSpace.h"
#include "odeSimpleSpace.h"
#include "odeSpace.h"
#include "odeQuadTreeSpace.h"

#ifndef CPPPARSER
extern Dtool_PyTypedObject Dtool_OdeHashSpace;
extern Dtool_PyTypedObject Dtool_OdeSimpleSpace;
extern Dtool_PyTypedObject Dtool_OdeSpace;
extern Dtool_PyTypedObject Dtool_OdeQuadTreeSpace;
#endif

PyObject *Extension<OdeSpace>::_python_callback = nullptr;

/**
 * Do a sort of pseudo-downcast on this space in order to expose its
 * specialized functions.
 */
PyObject *Extension<OdeSpace>::
convert() const {
  Dtool_PyTypedObject *class_type;
  OdeSpace *space;

  switch (_this->get_class()) {
  case OdeGeom::GC_simple_space:
    space = new OdeSimpleSpace(_this->get_id());
    class_type = &Dtool_OdeSimpleSpace;
    break;

  case OdeGeom::GC_hash_space:
    space = new OdeHashSpace(_this->get_id());
    class_type = &Dtool_OdeHashSpace;
    break;

  case OdeGeom::GC_quad_tree_space:
    space = new OdeQuadTreeSpace(_this->get_id());
    class_type = &Dtool_OdeQuadTreeSpace;
    break;

  default:
    // This shouldn't happen, but if it does, we should just return a regular
    // OdeSpace.
    space = new OdeSpace(_this->get_id());
    class_type = &Dtool_OdeSpace;
  }

  return DTool_CreatePyInstanceTyped((void *)space, *class_type,
                                     true, false, space->get_type_index());
}

int Extension<OdeSpace>::
collide(PyObject* arg, PyObject* callback) {
  nassertr(callback != nullptr, -1);

  if (!PyCallable_Check(callback)) {
    PyErr_Format(PyExc_TypeError, "'%s' object is not callable", callback->ob_type->tp_name);
    return -1;

  } else if (_this->get_id() == nullptr) {
    // Well, while we're in the mood of python exceptions, let's make this one
    // too.
    PyErr_Format(PyExc_TypeError, "OdeSpace is not valid!");
    return -1;

  } else {
    _python_callback = (PyObject*) callback;
    Py_XINCREF(_python_callback);
    dSpaceCollide(_this->get_id(), (void*) arg, &near_callback);
    Py_XDECREF(_python_callback);
    return 0;
  }
}

void Extension<OdeSpace>::
near_callback(void *data, dGeomID o1, dGeomID o2) {
  OdeGeom g1 (o1);
  OdeGeom g2 (o2);
  PyObject* p1 = invoke_extension(&g1).convert();
  PyObject* p2 = invoke_extension(&g2).convert();
  PyObject *result = PyObject_CallFunctionObjArgs(_python_callback, (PyObject*) data, p1, p2, nullptr);
  if (!result) {
    odespace_cat.error() << "An error occurred while calling python function!\n";
    PyErr_Print();
  } else {
    Py_DECREF(result);
  }
  Py_XDECREF(p2);
  Py_XDECREF(p1);
}

#endif  // HAVE_PYTHON
