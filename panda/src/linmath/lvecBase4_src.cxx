// Filename: lvecBase4_src.cxx
// Created by:  drose (08Mar00)
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

TypeHandle FLOATNAME(LVecBase4)::_type_handle;

#ifdef HAVE_PYTHON
#include "py_panda.h"  

#ifndef CPPPARSER
IMPORT_THIS struct Dtool_PyTypedObject FLOATNAME(Dtool_LVecBase2);
IMPORT_THIS struct Dtool_PyTypedObject FLOATNAME(Dtool_LVecBase3);
IMPORT_THIS struct Dtool_PyTypedObject FLOATNAME(Dtool_LVecBase4);
#endif
#endif  // HAVE_PYTHON

const FLOATNAME(LVecBase4) FLOATNAME(LVecBase4)::_zero =
  FLOATNAME(LVecBase4)(0.0f, 0.0f, 0.0f, 0.0f);
const FLOATNAME(LVecBase4) FLOATNAME(LVecBase4)::_unit_x =
  FLOATNAME(LVecBase4)(1.0f, 0.0f, 0.0f, 0.0f);
const FLOATNAME(LVecBase4) FLOATNAME(LVecBase4)::_unit_y =
  FLOATNAME(LVecBase4)(0.0f, 1.0f, 0.0f, 0.0f);
const FLOATNAME(LVecBase4) FLOATNAME(LVecBase4)::_unit_z =
  FLOATNAME(LVecBase4)(0.0f, 0.0f, 1.0f, 0.0f);
const FLOATNAME(LVecBase4) FLOATNAME(LVecBase4)::_unit_w =
  FLOATNAME(LVecBase4)(0.0f, 0.0f, 0.0f, 1.0f);

#ifdef HAVE_PYTHON
////////////////////////////////////////////////////////////////////
//     Function: LVecBase4::__reduce__
//       Access: Published
//  Description: This special Python method is implement to provide
//               support for the pickle module.
////////////////////////////////////////////////////////////////////
PyObject *FLOATNAME(LVecBase4)::
__reduce__(PyObject *self) const {
  // We should return at least a 2-tuple, (Class, (args)): the
  // necessary class object whose constructor we should call
  // (e.g. this), and the arguments necessary to reconstruct this
  // object.
  PyObject *this_class = PyObject_Type(self);
  if (this_class == NULL) {
    return NULL;
  }

  PyObject *result = Py_BuildValue("(O(ffff))", this_class, 
                                   (*this)[0], (*this)[1], (*this)[2], (*this)[3]);
  Py_DECREF(this_class);
  return result;
}

////////////////////////////////////////////////////////////////////
//     Function: LVecBase4::__getattr__
//       Access: Published
//  Description: This is used to implement swizzle masks.
////////////////////////////////////////////////////////////////////
PyObject *FLOATNAME(LVecBase4)::
__getattr__(const string &attr_name) const {
#ifndef NDEBUG
  // Validate the attribute name.
  for (string::const_iterator it = attr_name.begin(); it < attr_name.end(); it++) {
    if (*it < 'w' || *it > 'z') {
      return NULL;
    }
  }
#endif

  if (attr_name.size() == 1) {
    if (attr_name[0] == 'w') {
      return PyFloat_FromDouble(_v.data[3]);
    } else {
      return PyFloat_FromDouble(_v.data[attr_name[0] - 'x']);
    }

  } else if (attr_name.size() == 2) {
    FLOATNAME(LVecBase2) *vec = new FLOATNAME(LVecBase2);
    vec->_v.v._0 = _v.data[(attr_name[0] == 'w') ? 3 : attr_name[0] - 'x'];
    vec->_v.v._1 = _v.data[(attr_name[1] == 'w') ? 3 : attr_name[1] - 'x'];
    return DTool_CreatePyInstance((void *)vec, FLOATNAME(Dtool_LVecBase2), true, false);

  } else if (attr_name.size() == 3) {
    FLOATNAME(LVecBase3) *vec = new FLOATNAME(LVecBase3);
    vec->_v.v._0 = _v.data[(attr_name[0] == 'w') ? 3 : attr_name[0] - 'x'];
    vec->_v.v._1 = _v.data[(attr_name[1] == 'w') ? 3 : attr_name[1] - 'x'];
    vec->_v.v._2 = _v.data[(attr_name[2] == 'w') ? 3 : attr_name[2] - 'x'];
    return DTool_CreatePyInstance((void *)vec, FLOATNAME(Dtool_LVecBase3), true, false);

  } else if (attr_name.size() == 4) {
    FLOATNAME(LVecBase4) *vec = new FLOATNAME(LVecBase4);
    vec->_v.v._0 = _v.data[(attr_name[0] == 'w') ? 3 : attr_name[0] - 'x'];
    vec->_v.v._1 = _v.data[(attr_name[1] == 'w') ? 3 : attr_name[1] - 'x'];
    vec->_v.v._2 = _v.data[(attr_name[2] == 'w') ? 3 : attr_name[2] - 'x'];
    vec->_v.v._3 = _v.data[(attr_name[3] == 'w') ? 3 : attr_name[3] - 'x'];
    return DTool_CreatePyInstance((void *)vec, FLOATNAME(Dtool_LVecBase4), true, false);
  }

  return NULL;
}

////////////////////////////////////////////////////////////////////
//     Function: LVecBase4::__setattr__
//       Access: Published
//  Description: This is used to implement write masks.
////////////////////////////////////////////////////////////////////
int FLOATNAME(LVecBase4)::
__setattr__(PyObject *self, const string &attr_name, PyObject *assign) {
#ifndef NDEBUG
  // Validate the attribute name.
  for (string::const_iterator it = attr_name.begin(); it < attr_name.end(); it++) {
    if (*it < 'w' || *it > 'z') {
      return NULL;
    }
  }
#endif

  // It is a sequence, perhaps another vector?
  if (PySequence_Check(assign)) {

    // Whoosh.
    PyObject* fast = PySequence_Fast(assign, "");
    nassertr(fast != NULL, -1);
    
    // Let's be strict about size mismatches, to prevent user error.
    if (PySequence_Fast_GET_SIZE(fast) != attr_name.size()) {
      PyErr_SetString(PyExc_ValueError, "length mismatch");
      Py_DECREF(fast);
      return -1;
    }
    
    // Get a pointer to the items, iterate over it and
    // perform our magic assignment.  Fast fast.  Oh yeah.
    PyObject** items = PySequence_Fast_ITEMS(fast);
    for (int i = 0; i < attr_name.size(); ++i) {

      PyObject* fl = PyNumber_Float(items[i]);
      if (fl == NULL) {
        // Oh darn.  Not when we've come this far.
        PyErr_SetString(PyExc_ValueError, "a sequence of floats is required");
        Py_DECREF(fast);
        return -1;
      }
      double value = PyFloat_AS_DOUBLE(fl);
      Py_DECREF(fl);

      _v.data[(attr_name[i] == 'w') ? 3 : attr_name[i] - 'x'] = value;
    }

    Py_DECREF(fast);

  } else {
    // Maybe it's a single floating-point value.
    PyObject* fl = PyNumber_Float(assign);
    if (fl == NULL) {
      // It's not a floating-point value either?
      // Sheesh, I don't know what to do with it then.
      if (attr_name.size() == 1) {
        PyErr_SetString(PyExc_ValueError, "a float is required");
      } else {
        PyErr_Format(PyExc_ValueError, "'%.200s' object is not iterable",
          assign->ob_type->tp_name);
      }
      return -1;
    }
    double value = PyFloat_AS_DOUBLE(fl);
    Py_DECREF(fl);

    // Loop through the components in the attribute name,
    // and assign the floating-point value to every one of them.
    for (string::const_iterator it = attr_name.begin(); it < attr_name.end(); it++) {
      _v.data[((*it) == 'w') ? 3 : (*it) - 'x'] = value;
    } 
  }

  return 0;
}
#endif  // HAVE_PYTHON

////////////////////////////////////////////////////////////////////
//     Function: LVecBase4::init_type
//       Access: Public, Static
//  Description:
////////////////////////////////////////////////////////////////////
void FLOATNAME(LVecBase4)::
init_type() {
  if (_type_handle == TypeHandle::none()) {
    // Format a string to describe the type.
    string name = "LVecBase4";
    name += FLOATTOKEN;
    register_type(_type_handle, name);
  }
}
