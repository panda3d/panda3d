// Filename: lvector4_src.cxx
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

TypeHandle FLOATNAME(LVector4)::_type_handle;

#ifdef HAVE_PYTHON
#include "py_panda.h"  

#ifndef CPPPARSER
IMPORT_THIS struct Dtool_PyTypedObject FLOATNAME(Dtool_LVector2);
IMPORT_THIS struct Dtool_PyTypedObject FLOATNAME(Dtool_LVector3);
IMPORT_THIS struct Dtool_PyTypedObject FLOATNAME(Dtool_LVector4);
#endif
#endif  // HAVE_PYTHON

#ifdef HAVE_PYTHON
////////////////////////////////////////////////////////////////////
//     Function: LVector4::__getattr__
//       Access: Published
//  Description: This is used to implement swizzle masks.
////////////////////////////////////////////////////////////////////
PyObject *FLOATNAME(LVector4)::
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
    FLOATNAME(LVector2) *vec = new FLOATNAME(LVector2);
    vec->_v.v._0 = _v.data[(attr_name[0] == 'w') ? 3 : attr_name[0] - 'x'];
    vec->_v.v._1 = _v.data[(attr_name[1] == 'w') ? 3 : attr_name[1] - 'x'];
    return DTool_CreatePyInstance((void *)vec, FLOATNAME(Dtool_LVector2), true, false);

  } else if (attr_name.size() == 3) {
    FLOATNAME(LVector3) *vec = new FLOATNAME(LVector3);
    vec->_v.v._0 = _v.data[(attr_name[0] == 'w') ? 3 : attr_name[0] - 'x'];
    vec->_v.v._1 = _v.data[(attr_name[1] == 'w') ? 3 : attr_name[1] - 'x'];
    vec->_v.v._2 = _v.data[(attr_name[2] == 'w') ? 3 : attr_name[2] - 'x'];
    return DTool_CreatePyInstance((void *)vec, FLOATNAME(Dtool_LVector3), true, false);

  } else if (attr_name.size() == 4) {
    FLOATNAME(LVector4) *vec = new FLOATNAME(LVector4);
    vec->_v.v._0 = _v.data[(attr_name[0] == 'w') ? 3 : attr_name[0] - 'x'];
    vec->_v.v._1 = _v.data[(attr_name[1] == 'w') ? 3 : attr_name[1] - 'x'];
    vec->_v.v._2 = _v.data[(attr_name[2] == 'w') ? 3 : attr_name[2] - 'x'];
    vec->_v.v._3 = _v.data[(attr_name[3] == 'w') ? 3 : attr_name[3] - 'x'];
    return DTool_CreatePyInstance((void *)vec, FLOATNAME(Dtool_LVector4), true, false);
  }

  return NULL;
}

////////////////////////////////////////////////////////////////////
//     Function: LVector4::__setattr__
//       Access: Published
//  Description: This is used to implement write masks.
////////////////////////////////////////////////////////////////////
int FLOATNAME(LVector4)::
__setattr__(PyObject *self, const string &attr_name, PyObject *assign) {
  return FLOATNAME(LVecBase4)::__setattr__(self, attr_name, assign);
}
#endif  // HAVE_PYTHON

////////////////////////////////////////////////////////////////////
//     Function: LVector2::init_type
//       Access: Public, Static
//  Description:
////////////////////////////////////////////////////////////////////
void FLOATNAME(LVector4)::
init_type() {
  if (_type_handle == TypeHandle::none()) {
    FLOATNAME(LVecBase4)::init_type();
    string name = "LVector4";
    name += FLOATTOKEN;
    register_type(_type_handle, name,
                  FLOATNAME(LVecBase4)::get_class_type());
  }
}


