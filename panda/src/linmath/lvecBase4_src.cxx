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
__setattr__(PyObject *self, const string &attr_name, FLOATTYPE val) {
#ifndef NDEBUG
  // Loop through the components in the attribute name,
  // and assign the floating-point value to every one of them.
  for (string::const_iterator it = attr_name.begin(); it < attr_name.end(); it++) {
    if ((*it) < 'w' || (*it) > 'z') {
      PyTypeObject *tp = self->ob_type;
      PyErr_Format(PyExc_AttributeError,
                   "'%.100s' object has no attribute '%.200s'",
                   tp->tp_name, attr_name.c_str());
      return -1;
    }

    _v.data[((*it) == 'w') ? 3 : (*it) - 'x'] = val;
  }
#endif

  return 0;
}

////////////////////////////////////////////////////////////////////
//     Function: LVecBase4::__setattr__
//       Access: Published
//  Description: This is used to implement write masks.
////////////////////////////////////////////////////////////////////
int FLOATNAME(LVecBase4)::
__setattr__(PyObject *self, const string &attr_name, FLOATNAME(LVecBase2) val) {
#ifndef NDEBUG
  // Validate the attribute name.
  if (attr_name.size() > 4) {
    goto attrerr;
  }
  for (string::const_iterator it = attr_name.begin(); it < attr_name.end(); it++) {
    if ((*it) < 'w' || (*it) > 'z') {
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
  _v.data[(attr_name[0] == 'w') ? 3 : attr_name[0] - 'x'] = val._v.v._0;
  _v.data[(attr_name[1] == 'w') ? 3 : attr_name[1] - 'x'] = val._v.v._1;
  return 0;

attrerr:
  PyTypeObject *tp = self->ob_type;
  PyErr_Format(PyExc_AttributeError,
               "'%.100s' object has no attribute '%.200s'",
               tp->tp_name, attr_name.c_str());
  return -1;
}

////////////////////////////////////////////////////////////////////
//     Function: LVecBase4::__setattr__
//       Access: Published
//  Description: This is used to implement write masks.
////////////////////////////////////////////////////////////////////
int FLOATNAME(LVecBase4)::
__setattr__(PyObject *self, const string &attr_name, FLOATNAME(LVecBase3) val) {
#ifndef NDEBUG
  // Validate the attribute name.
  if (attr_name.size() > 4) {
    goto attrerr;
  }
  for (string::const_iterator it = attr_name.begin(); it < attr_name.end(); it++) {
    if ((*it) < 'w' || (*it) > 'z') {
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
  _v.data[(attr_name[0] == 'w') ? 3 : attr_name[0] - 'x'] = val._v.v._0;
  _v.data[(attr_name[1] == 'w') ? 3 : attr_name[1] - 'x'] = val._v.v._1;
  _v.data[(attr_name[2] == 'w') ? 3 : attr_name[2] - 'x'] = val._v.v._2;
  return 0;

attrerr:
  PyTypeObject *tp = self->ob_type;
  PyErr_Format(PyExc_AttributeError,
               "'%.100s' object has no attribute '%.200s'",
               tp->tp_name, attr_name.c_str());
  return -1;
}

////////////////////////////////////////////////////////////////////
//     Function: LVecBase4::__setattr__
//       Access: Published
//  Description: This is used to implement write masks.
////////////////////////////////////////////////////////////////////
int FLOATNAME(LVecBase4)::
__setattr__(PyObject *self, const string &attr_name, FLOATNAME(LVecBase4) val) {
#ifndef NDEBUG
  // Validate the attribute name.
  if (attr_name.size() > 4) {
    goto attrerr;
  }
  for (string::const_iterator it = attr_name.begin(); it < attr_name.end(); it++) {
    if ((*it) < 'w' || (*it) > 'z') {
      goto attrerr;
    }
  }
  
  // Make sure the right type is passed for the right write mask length.
  if (attr_name.size() == 1) {
    PyErr_SetString(PyExc_AttributeError, "a float is required");
    return -1;

  } else if (attr_name.size() != 4) {
    PyTypeObject &tp = FLOATNAME(Dtool_LVecBase4).As_PyTypeObject();
    PyErr_Format(PyExc_AttributeError,
                 "cannot assign '%s' to write mask '%s'",
                 tp.tp_name, attr_name.c_str());
    return -1;
  }
#endif

  // Assign the components.
  _v.data[(attr_name[0] == 'w') ? 3 : attr_name[0] - 'x'] = val._v.v._0;
  _v.data[(attr_name[1] == 'w') ? 3 : attr_name[1] - 'x'] = val._v.v._1;
  _v.data[(attr_name[2] == 'w') ? 3 : attr_name[2] - 'x'] = val._v.v._2;
  _v.data[(attr_name[3] == 'w') ? 3 : attr_name[3] - 'x'] = val._v.v._3;
  return 0;

attrerr:
  PyTypeObject *tp = self->ob_type;
  PyErr_Format(PyExc_AttributeError,
               "'%.100s' object has no attribute '%.200s'",
               tp->tp_name, attr_name.c_str());
  return -1;
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
