/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file odeUtil_ext.cxx
 * @author rdb
 * @date 2013-12-10
 */

#include "odeUtil_ext.h"
#include "config_ode.h"
#include "odeGeom.h"
#include "odeGeom_ext.h"

#ifdef HAVE_PYTHON

PyObject *Extension<OdeUtil>::_python_callback = nullptr;

/**
 * Calls the callback for all potentially intersecting pairs that contain one
 * geom from geom1 and one geom from geom2.
 */
int Extension<OdeUtil>::
collide2(const OdeGeom &geom1, const OdeGeom &geom2, PyObject* arg, PyObject* callback) {
  nassertr(callback != nullptr, -1);
  if (!PyCallable_Check(callback)) {
    PyErr_Format(PyExc_TypeError, "'%s' object is not callable", callback->ob_type->tp_name);
    return -1;
  } else {
    _python_callback = (PyObject*) callback;
    Py_XINCREF(_python_callback);
    dSpaceCollide2(geom1.get_id(), geom2.get_id(), (void*) arg, &near_callback);
    Py_XDECREF(_python_callback);
    return 0;
  }
}

void Extension<OdeUtil>::
near_callback(void *data, dGeomID o1, dGeomID o2) {
  if (ode_cat.is_spam()) {
    ode_cat.spam()
      << "near_callback called, data: " << data << ", dGeomID1: " << o1 << ", dGeomID2: " << o2 << "\n";
  }

  OdeGeom g1 (o1);
  OdeGeom g2 (o2);
  PyObject* p1 = invoke_extension(&g1).convert();
  PyObject* p2 = invoke_extension(&g2).convert();
  PyObject* result = PyObject_CallFunctionObjArgs(_python_callback, (PyObject*) data, p1, p2, nullptr);
  if (!result) {
    ode_cat.error() << "An error occurred while calling python function!\n";
    PyErr_Print();
  }
  Py_XDECREF(p1);
  Py_XDECREF(p2);
}

#endif  // HAVE_PYTHON
