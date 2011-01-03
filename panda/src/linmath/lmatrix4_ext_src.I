// Filename: lmatrix4_ext_src.I
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


////////////////////////////////////////////////////////////////////
//     Function: LMatrix4::Row::__setitem__
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
INLINE void EXT_NESTED_METHOD_ARGS(FLOATNAME(LMatrix4), Row,
__setitem__, int i, FLOATTYPE v) {
  nassertv(i >= 0 && i < 4);
  this->_row[i] = v;
}

////////////////////////////////////////////////////////////////////
//     Function: LMatrix4::__reduce__
//       Access: Published
//  Description: This special Python method is implement to provide
//               support for the pickle module.
////////////////////////////////////////////////////////////////////
PyObject *EXT_CONST_METHOD_ARGS(FLOATNAME(LMatrix4),
__reduce__, PyObject *self) {
  // We should return at least a 2-tuple, (Class, (args)): the
  // necessary class object whose constructor we should call
  // (e.g. this), and the arguments necessary to reconstruct this
  // object.
  PyObject *this_class = PyObject_Type(self);
  if (this_class == NULL) {
    return NULL;
  }

  PyObject *result = Py_BuildValue("(O(ffffffffffffffff))", this_class, 
    this->_m.m._00, this->_m.m._01, this->_m.m._02, this->_m.m._03,
    this->_m.m._10, this->_m.m._11, this->_m.m._12, this->_m.m._13,
    this->_m.m._20, this->_m.m._21, this->_m.m._22, this->_m.m._23,
    this->_m.m._30, this->_m.m._31, this->_m.m._32, this->_m.m._33);

  Py_DECREF(this_class);
  return result;
}

////////////////////////////////////////////////////////////////////
//     Function: LMatrix4::python_repr
//       Access: Published
//  Description:
////////////////////////////////////////////////////////////////////
void EXT_CONST_METHOD_ARGS(FLOATNAME(LMatrix4),
python_repr, ostream &out, const string &class_name) {
  out << class_name << "(" 
      << MAYBE_ZERO(this->_m.m._00) << ", "
      << MAYBE_ZERO(this->_m.m._01) << ", "
      << MAYBE_ZERO(this->_m.m._02) << ", "
      << MAYBE_ZERO(this->_m.m._03) << ", "

      << MAYBE_ZERO(this->_m.m._10) << ", "
      << MAYBE_ZERO(this->_m.m._11) << ", "
      << MAYBE_ZERO(this->_m.m._12) << ", "
      << MAYBE_ZERO(this->_m.m._13) << ", "

      << MAYBE_ZERO(this->_m.m._20) << ", "
      << MAYBE_ZERO(this->_m.m._21) << ", "
      << MAYBE_ZERO(this->_m.m._22) << ", "
      << MAYBE_ZERO(this->_m.m._23) << ", "

      << MAYBE_ZERO(this->_m.m._30) << ", "
      << MAYBE_ZERO(this->_m.m._31) << ", "
      << MAYBE_ZERO(this->_m.m._32) << ", "
      << MAYBE_ZERO(this->_m.m._33) << ")";
}

