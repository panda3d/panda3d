// Filename: lvector3_src.cxx
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

TypeHandle FLOATNAME(LVector3)::_type_handle;

#ifdef HAVE_PYTHON
#include "py_panda.h"  

#ifndef CPPPARSER
IMPORT_THIS struct Dtool_PyTypedObject FLOATNAME(Dtool_LVector2);
IMPORT_THIS struct Dtool_PyTypedObject FLOATNAME(Dtool_LVector3);
#endif
#endif  // HAVE_PYTHON

#ifdef HAVE_PYTHON
////////////////////////////////////////////////////////////////////
//     Function: LVector3::__getattr__
//       Access: Published
//  Description: This is used to implement swizzle masks.
////////////////////////////////////////////////////////////////////
PyObject *FLOATNAME(LVector3)::
__getattr__(const string &attr_name) const {
#ifndef NDEBUG
  // Validate the attribute name.
  for (string::const_iterator it = attr_name.begin(); it < attr_name.end(); it++) {
    if (*it < 'x' || *it > 'z') {
      return NULL;
    }
  }
#endif

  if (attr_name.size() == 1) {
    return PyFloat_FromDouble(_v.data[attr_name[0] - 'x']);

  } else if (attr_name.size() == 2) {
    FLOATNAME(LVector2) *vec = new FLOATNAME(LVector2);
    vec->_v.v._0 = _v.data[attr_name[0] - 'x'];
    vec->_v.v._1 = _v.data[attr_name[1] - 'x'];
    return DTool_CreatePyInstance((void *)vec, FLOATNAME(Dtool_LVector2), true, false);

  } else if (attr_name.size() == 3) {
    FLOATNAME(LVector3) *vec = new FLOATNAME(LVector3);
    vec->_v.v._0 = _v.data[attr_name[0] - 'x'];
    vec->_v.v._1 = _v.data[attr_name[1] - 'x'];
    vec->_v.v._2 = _v.data[attr_name[2] - 'x'];
    return DTool_CreatePyInstance((void *)vec, FLOATNAME(Dtool_LVector3), true, false);
  }

  return NULL;
}

////////////////////////////////////////////////////////////////////
//     Function: LVector3::__setattr__
//       Access: Published
//  Description: This is used to implement write masks.
////////////////////////////////////////////////////////////////////
int FLOATNAME(LVector3)::
__setattr__(PyObject *self, const string &attr_name, FLOATTYPE val) {
#ifndef NDEBUG
  // Loop through the components in the attribute name,
  // and assign the floating-point value to every one of them.
  for (string::const_iterator it = attr_name.begin(); it < attr_name.end(); it++) {
    if ((*it) < 'x' || (*it) > 'z') {
      PyTypeObject *tp = Py_TYPE(self);
      PyErr_Format(PyExc_AttributeError,
                   "'%.100s' object has no attribute '%.200s'",
                   tp->tp_name, attr_name.c_str());
      return -1;
    }

    _v.data[(*it) - 'x'] = val;
  }
#endif

  return 0;
}

////////////////////////////////////////////////////////////////////
//     Function: LVector3::__setattr__
//       Access: Published
//  Description: This is used to implement write masks.
////////////////////////////////////////////////////////////////////
int FLOATNAME(LVector3)::
__setattr__(PyObject *self, const string &attr_name, FLOATNAME(LVecBase2) val) {
#ifndef NDEBUG
  // Validate the attribute name.
  if (attr_name.size() > 3) {
    goto attrerr;
  }
  for (string::const_iterator it = attr_name.begin(); it < attr_name.end(); it++) {
    if ((*it) < 'x' || (*it) > 'z') {
      goto attrerr;
    }
  }
  
  // Make sure the right type is passed for the right write mask length.
  if (attr_name.size() == 1) {
    PyErr_SetString(PyExc_AttributeError, "a float is required");
    return -1;

  } else if (attr_name.size() != 2) {
    PyTypeObject &tp = FLOATNAME(Dtool_LVecBase2).As_PyTypeObject();
    PyErr_Format(PyExc_AttributeError,
                 "cannot assign '%s' to write mask '%s'",
                 tp.tp_name, attr_name.c_str());
    return -1;
  }
#endif

  // Assign the components.
  _v.data[attr_name[0] - 'x'] = val._v.v._0;
  _v.data[attr_name[1] - 'x'] = val._v.v._1;
  return 0;

attrerr:
  PyTypeObject *tp = Py_TYPE(self);
  PyErr_Format(PyExc_AttributeError,
               "'%.100s' object has no attribute '%.200s'",
               tp->tp_name, attr_name.c_str());
  return -1;
}

////////////////////////////////////////////////////////////////////
//     Function: LVector3::__setattr__
//       Access: Published
//  Description: This is used to implement write masks.
////////////////////////////////////////////////////////////////////
int FLOATNAME(LVector3)::
__setattr__(PyObject *self, const string &attr_name, FLOATNAME(LVecBase3) val) {
#ifndef NDEBUG
  // Validate the attribute name.
  if (attr_name.size() > 3) {
    goto attrerr;
  }
  for (string::const_iterator it = attr_name.begin(); it < attr_name.end(); it++) {
    if ((*it) < 'x' || (*it) > 'z') {
      goto attrerr;
    }
  }
  
  // Make sure the right type is passed for the right write mask length.
  if (attr_name.size() == 1) {
    PyErr_SetString(PyExc_AttributeError, "a float is required");
    return -1;

  } else if (attr_name.size() != 3) {
    PyTypeObject &tp = FLOATNAME(Dtool_LVecBase3).As_PyTypeObject();
    PyErr_Format(PyExc_AttributeError,
                 "cannot assign '%s' to write mask '%s'",
                 tp.tp_name, attr_name.c_str());
    return -1;
  }
#endif

  // Assign the components.
  _v.data[attr_name[0] - 'x'] = val._v.v._0;
  _v.data[attr_name[1] - 'x'] = val._v.v._1;
  _v.data[attr_name[2] - 'x'] = val._v.v._2;
  return 0;

attrerr:
  PyTypeObject *tp = Py_TYPE(self);
  PyErr_Format(PyExc_AttributeError,
               "'%.100s' object has no attribute '%.200s'",
               tp->tp_name, attr_name.c_str());
  return -1;
}
#endif  // HAVE_PYTHON

////////////////////////////////////////////////////////////////////
//     Function: LVector3::init_type
//       Access: Public, Static
//  Description:
////////////////////////////////////////////////////////////////////
void FLOATNAME(LVector3)::
init_type() {
  if (_type_handle == TypeHandle::none()) {
    FLOATNAME(LVecBase3)::init_type();
    string name = "LVector3";
    name += FLOATTOKEN;
    register_type(_type_handle, name,
                  FLOATNAME(LVecBase3)::get_class_type());
  }
}


