// Filename: lpoint2_ext_src.I
// Created by:  rdb (02Jan11)
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


#ifndef CPPPARSER
IMPORT_THIS struct Dtool_PyTypedObject FLOATNAME(Dtool_LPoint2);
#endif

////////////////////////////////////////////////////////////////////
//     Function: LPoint2::python_repr
//       Access: Published
//  Description:
////////////////////////////////////////////////////////////////////
INLINE void EXT_CONST_METHOD_ARGS(FLOATNAME(LPoint2),
python_repr, ostream &out, const string &class_name) {
  out << class_name << "("
      << MAYBE_ZERO(this->_v.v._0) << ", "
      << MAYBE_ZERO(this->_v.v._1) << ")";
}

////////////////////////////////////////////////////////////////////
//     Function: LPoint2::__getattr__
//       Access: Published
//  Description: This is used to implement swizzle masks.
////////////////////////////////////////////////////////////////////
INLINE PyObject *EXT_CONST_METHOD_ARGS(FLOATNAME(LPoint2),
__getattr__, const string &attr_name) {
#ifndef NDEBUG
  // Validate the attribute name.
  for (string::const_iterator it = attr_name.begin(); it < attr_name.end(); it++) {
    if (*it != 'x' && *it != 'y') {
      return NULL;
    }
  }
#endif

  if (attr_name.size() == 1) {
    return PyFloat_FromDouble(this->_v.data[attr_name[0] - 'x']);

  } else if (attr_name.size() == 2) {
    FLOATNAME(LPoint2) *vec = new FLOATNAME(LPoint2);
    vec->_v.v._0 = this->_v.data[attr_name[0] - 'x'];
    vec->_v.v._1 = this->_v.data[attr_name[1] - 'x'];
    return DTool_CreatePyInstance((void *)vec, FLOATNAME(Dtool_LPoint2), true, false);
  }

  return NULL;
}

int EXT_METHOD_ARGS(FLOATNAME(LVecBase2), __setattr__, PyObject*, const string&, PyObject*);

////////////////////////////////////////////////////////////////////
//     Function: LPoint2::__setattr__
//       Access: Published
//  Description: This is used to implement write masks.
////////////////////////////////////////////////////////////////////
INLINE int EXT_METHOD_ARGS(FLOATNAME(LPoint2),
__setattr__, PyObject *self, const string &attr_name, PyObject *assign) {
  return CALL_EXT_METHOD(FLOATNAME(LVecBase2), __setattr__, this, self, attr_name, assign);
}

