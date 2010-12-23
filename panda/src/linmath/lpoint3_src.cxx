// Filename: lpoint3_src.cxx
// Created by:  drose (25Sep99)
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

TypeHandle FLOATNAME(LPoint3)::_type_handle;

#ifdef HAVE_PYTHON
#include "py_panda.h"  

#ifndef CPPPARSER
IMPORT_THIS struct Dtool_PyTypedObject FLOATNAME(Dtool_LPoint2);
IMPORT_THIS struct Dtool_PyTypedObject FLOATNAME(Dtool_LPoint3);
#endif
#endif  // HAVE_PYTHON

#ifdef HAVE_PYTHON
////////////////////////////////////////////////////////////////////
//     Function: LPoint3::__getattr__
//       Access: Published
//  Description: This is used to implement swizzle masks.
////////////////////////////////////////////////////////////////////
PyObject *FLOATNAME(LPoint3)::
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
    FLOATNAME(LPoint2) *vec = new FLOATNAME(LPoint2);
    vec->_v.v._0 = _v.data[attr_name[0] - 'x'];
    vec->_v.v._1 = _v.data[attr_name[1] - 'x'];
    return DTool_CreatePyInstance((void *)vec, FLOATNAME(Dtool_LPoint2), true, false);

  } else if (attr_name.size() == 3) {
    FLOATNAME(LPoint3) *vec = new FLOATNAME(LPoint3);
    vec->_v.v._0 = _v.data[attr_name[0] - 'x'];
    vec->_v.v._1 = _v.data[attr_name[1] - 'x'];
    vec->_v.v._2 = _v.data[attr_name[2] - 'x'];
    return DTool_CreatePyInstance((void *)vec, FLOATNAME(Dtool_LPoint3), true, false);
  }

  return NULL;
}

////////////////////////////////////////////////////////////////////
//     Function: LPoint3::__setattr__
//       Access: Published
//  Description: This is used to implement write masks.
////////////////////////////////////////////////////////////////////
int FLOATNAME(LPoint3)::
__setattr__(PyObject *self, const string &attr_name, PyObject *assign) {
  return FLOATNAME(LVecBase3)::__setattr__(self, attr_name, assign);
}
#endif  // HAVE_PYTHON

////////////////////////////////////////////////////////////////////
//     Function: LPoint3::init_type
//       Access: Public, Static
//  Description:
////////////////////////////////////////////////////////////////////
void FLOATNAME(LPoint3)::
init_type() {
  if (_type_handle == TypeHandle::none()) {
    FLOATNAME(LVecBase3)::init_type();
    string name = "LPoint3";
    name += FLOATTOKEN;
    register_type(_type_handle, name,
                  FLOATNAME(LVecBase3)::get_class_type());
  }
}

