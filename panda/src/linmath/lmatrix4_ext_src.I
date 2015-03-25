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
//     Function: LMatrix4::__reduce__
//       Access: Published
//  Description: This special Python method is implement to provide
//               support for the pickle module.
////////////////////////////////////////////////////////////////////
INLINE_LINMATH PyObject *Extension<FLOATNAME(LMatrix4)>::
__reduce__(PyObject *self) const {
  // We should return at least a 2-tuple, (Class, (args)): the
  // necessary class object whose constructor we should call
  // (e.g. this), and the arguments necessary to reconstruct this
  // object.
  PyObject *this_class = PyObject_Type(self);
  if (this_class == NULL) {
    return NULL;
  }

  PyObject *result = Py_BuildValue("(O(ffffffffffffffff))", this_class, 
    _this->_m(0, 0), _this->_m(0, 1), _this->_m(0, 2), _this->_m(0, 3),
    _this->_m(1, 0), _this->_m(1, 1), _this->_m(1, 2), _this->_m(1, 3),
    _this->_m(2, 0), _this->_m(2, 1), _this->_m(2, 2), _this->_m(2, 3),
    _this->_m(3, 0), _this->_m(3, 1), _this->_m(3, 2), _this->_m(3, 3));

  Py_DECREF(this_class);
  return result;
}

////////////////////////////////////////////////////////////////////
//     Function: LMatrix4::__repr__
//       Access: Published
//  Description:
////////////////////////////////////////////////////////////////////
INLINE_LINMATH string Extension<FLOATNAME(LMatrix4)>::
__repr__() const {
  ostringstream out;
  out << "LMatrix4" << FLOATTOKEN << "("
      << MAYBE_ZERO(_this->_m(0, 0)) << ", "
      << MAYBE_ZERO(_this->_m(0, 1)) << ", "
      << MAYBE_ZERO(_this->_m(0, 2)) << ", "
      << MAYBE_ZERO(_this->_m(0, 3)) << ", "

      << MAYBE_ZERO(_this->_m(1, 0)) << ", "
      << MAYBE_ZERO(_this->_m(1, 1)) << ", "
      << MAYBE_ZERO(_this->_m(1, 2)) << ", "
      << MAYBE_ZERO(_this->_m(1, 3)) << ", "

      << MAYBE_ZERO(_this->_m(2, 0)) << ", "
      << MAYBE_ZERO(_this->_m(2, 1)) << ", "
      << MAYBE_ZERO(_this->_m(2, 2)) << ", "
      << MAYBE_ZERO(_this->_m(2, 3)) << ", "

      << MAYBE_ZERO(_this->_m(3, 0)) << ", "
      << MAYBE_ZERO(_this->_m(3, 1)) << ", "
      << MAYBE_ZERO(_this->_m(3, 2)) << ", "
      << MAYBE_ZERO(_this->_m(3, 3)) << ")";
  return out.str();
}
