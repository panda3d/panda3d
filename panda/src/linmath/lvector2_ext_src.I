// Filename: lvector2_ext_src.I
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
IMPORT_THIS struct Dtool_PyTypedObject FLOATNAME(Dtool_LVector2);
IMPORT_THIS struct Dtool_PyTypedObject FLOATNAME(Dtool_LVector3);
IMPORT_THIS struct Dtool_PyTypedObject FLOATNAME(Dtool_LVector4);
#endif

////////////////////////////////////////////////////////////////////
//     Function: LVector2::python_repr
//       Access: Published
//  Description:
////////////////////////////////////////////////////////////////////
INLINE_LINMATH void Extension<FLOATNAME(LVector2)>::
python_repr(ostream &out, const string &class_name) const {
  out << class_name << "("
      << MAYBE_ZERO(_this->_v(0)) << ", "
      << MAYBE_ZERO(_this->_v(1)) << ")";
}

////////////////////////////////////////////////////////////////////
//     Function: LVector2::__getattr__
//       Access: Published
//  Description: This is used to implement swizzle masks.
////////////////////////////////////////////////////////////////////
INLINE_LINMATH PyObject *Extension<FLOATNAME(LVector2)>::
__getattr__(const string &attr_name) const {
  // Validate the attribute name.
  for (string::const_iterator it = attr_name.begin(); it < attr_name.end(); it++) {
    if (*it != 'x' && *it != 'y') {
      return NULL;
    }
  }

  switch (attr_name.size()) {
    case 1:
      return PyFloat_FromDouble(_this->_v(attr_name[0] - 'x'));

    case 2: {
      FLOATNAME(LVector2) *vec = new FLOATNAME(LVector2);
      vec->_v(0) = _this->_v(attr_name[0] - 'x');
      vec->_v(1) = _this->_v(attr_name[1] - 'x');
      return DTool_CreatePyInstance((void *)vec, FLOATNAME(Dtool_LVector2), true, false);

    } case 3: {
      FLOATNAME(LVector3) *vec = new FLOATNAME(LVector3);
      vec->_v(0) = _this->_v(attr_name[0] - 'x');
      vec->_v(1) = _this->_v(attr_name[1] - 'x');
      vec->_v(2) = _this->_v(attr_name[2] - 'x');
      return DTool_CreatePyInstance((void *)vec, FLOATNAME(Dtool_LVector3), true, false);

    } case 4: {
      FLOATNAME(LVector4) *vec = new FLOATNAME(LVector4);
      vec->_v(0) = _this->_v(attr_name[0] - 'x');
      vec->_v(1) = _this->_v(attr_name[1] - 'x');
      vec->_v(2) = _this->_v(attr_name[2] - 'x');
      vec->_v(3) = _this->_v(attr_name[3] - 'x');
      return DTool_CreatePyInstance((void *)vec, FLOATNAME(Dtool_LVector4), true, false);
    }
  }

  return NULL;
}

////////////////////////////////////////////////////////////////////
//     Function: LVector2::__setattr__
//       Access: Published
//  Description: This is used to implement write masks.
////////////////////////////////////////////////////////////////////
INLINE_LINMATH int Extension<FLOATNAME(LVector2)>::
__setattr__(PyObject *self, const string &attr_name, PyObject *assign) {
  // Upcall to LVecBase2.
  return invoke_extension<FLOATNAME(LVecBase2)>(_this).__setattr__(self, attr_name, assign);
}
