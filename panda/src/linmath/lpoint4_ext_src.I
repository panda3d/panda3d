// Filename: lpoint4_ext_src.I
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
IMPORT_THIS struct Dtool_PyTypedObject FLOATNAME(Dtool_LPoint3);
IMPORT_THIS struct Dtool_PyTypedObject FLOATNAME(Dtool_LPoint4);
#endif

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
//     Function: LPoint4::python_repr
//       Access: Published
//  Description:
////////////////////////////////////////////////////////////////////
INLINE_LINMATH void Extension<FLOATNAME(LPoint4)>::
python_repr(ostream &out, const string &class_name) const {
  out << class_name << "("
      << MAYBE_ZERO(_this->_v(0)) << ", "
      << MAYBE_ZERO(_this->_v(1)) << ", "
      << MAYBE_ZERO(_this->_v(2)) << ", "
      << MAYBE_ZERO(_this->_v(3)) << ")";
}

////////////////////////////////////////////////////////////////////
//     Function: LPoint4::__getattr__
//       Access: Published
//  Description: This is used to implement swizzle masks.
////////////////////////////////////////////////////////////////////
INLINE_LINMATH PyObject *Extension<FLOATNAME(LPoint4)>::
__getattr__(const string &attr_name) const {
  // Validate the attribute name.
  for (string::const_iterator it = attr_name.begin(); it < attr_name.end(); it++) {
    if (*it < 'w' || *it > 'z') {
      return NULL;
    }
  }

  switch (attr_name.size()) {
    case 1:
      if (attr_name[0] == 'w') {
        return PY_FROM_FLOATTYPE(_this->_v(3));
      } else {
        return PY_FROM_FLOATTYPE(_this->_v(attr_name[0] - 'x'));
      }

    case 2: {
      FLOATNAME(LPoint2) *vec = new FLOATNAME(LPoint2);
      vec->_v(0) = _this->_v((attr_name[0] == 'w') ? 3 : attr_name[0] - 'x');
      vec->_v(1) = _this->_v((attr_name[1] == 'w') ? 3 : attr_name[1] - 'x');
      return DTool_CreatePyInstance((void *)vec, FLOATNAME(Dtool_LPoint2), true, false);

    } case 3: {
      FLOATNAME(LPoint3) *vec = new FLOATNAME(LPoint3);
      vec->_v(0) = _this->_v((attr_name[0] == 'w') ? 3 : attr_name[0] - 'x');
      vec->_v(1) = _this->_v((attr_name[1] == 'w') ? 3 : attr_name[1] - 'x');
      vec->_v(2) = _this->_v((attr_name[2] == 'w') ? 3 : attr_name[2] - 'x');
      return DTool_CreatePyInstance((void *)vec, FLOATNAME(Dtool_LPoint3), true, false);

    } case 4: {
      FLOATNAME(LPoint4) *vec = new FLOATNAME(LPoint4);
      vec->_v(0) = _this->_v((attr_name[0] == 'w') ? 3 : attr_name[0] - 'x');
      vec->_v(1) = _this->_v((attr_name[1] == 'w') ? 3 : attr_name[1] - 'x');
      vec->_v(2) = _this->_v((attr_name[2] == 'w') ? 3 : attr_name[2] - 'x');
      vec->_v(3) = _this->_v((attr_name[3] == 'w') ? 3 : attr_name[3] - 'x');
      return DTool_CreatePyInstance((void *)vec, FLOATNAME(Dtool_LPoint4), true, false);
    }
  }

  return NULL;
}

////////////////////////////////////////////////////////////////////
//     Function: LPoint4::__setattr__
//       Access: Published
//  Description: This is used to implement write masks.
////////////////////////////////////////////////////////////////////
INLINE_LINMATH int Extension<FLOATNAME(LPoint4)>::
__setattr__(PyObject *self, const string &attr_name, PyObject *assign) {
  // Upcall to LVecBase4.
  return invoke_extension<FLOATNAME(LVecBase4)>(_this).__setattr__(self, attr_name, assign);
}

#undef PY_FROM_FLOATTYPE
