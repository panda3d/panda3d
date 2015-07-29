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

#ifdef FLOATTYPE_IS_INT
#if PY_MAJOR_VERSION >= 3
#define PY_FROM_FLOATTYPE PyLong_FromLong
#else
#define PY_FROM_FLOATTYPE PyInt_FromLong
#endif
#else
#define PY_FROM_FLOATTYPE PyFloat_FromDouble
#endif

////////////////////////////////////////////////////////////////////
//     Function: LVector2::__repr__
//       Access: Published
//  Description:
////////////////////////////////////////////////////////////////////
INLINE_LINMATH string Extension<FLOATNAME(LVector2)>::
__repr__() const {
  ostringstream out;
  out << "LVector2" << FLOATTOKEN << "("
      << MAYBE_ZERO(_this->_v(0)) << ", "
      << MAYBE_ZERO(_this->_v(1)) << ")";
  return out.str();
}

////////////////////////////////////////////////////////////////////
//     Function: LVector2::__getattr__
//       Access: Published
//  Description: This is used to implement swizzle masks.
////////////////////////////////////////////////////////////////////
INLINE_LINMATH PyObject *Extension<FLOATNAME(LVector2)>::
__getattr__(PyObject *self, const string &attr_name) const {
#ifndef CPPPARSER
  extern struct Dtool_PyTypedObject FLOATNAME(Dtool_LVector2);
  extern struct Dtool_PyTypedObject FLOATNAME(Dtool_LVector3);
  extern struct Dtool_PyTypedObject FLOATNAME(Dtool_LVector4);
#endif

  // Validate the attribute name.
  for (string::const_iterator it = attr_name.begin(); it < attr_name.end(); it++) {
    if (*it != 'x' && *it != 'y') {
      return Dtool_Raise_AttributeError(self, attr_name.c_str());
    }
  }

  switch (attr_name.size()) {
    case 1:
      return PY_FROM_FLOATTYPE(_this->_v(attr_name[0] - 'x'));

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

  return Dtool_Raise_AttributeError(self, attr_name.c_str());
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

#undef PY_FROM_FLOATTYPE
