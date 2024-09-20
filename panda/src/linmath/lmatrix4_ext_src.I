/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file lmatrix4_ext_src.I
 * @author rdb
 * @date 2011-01-02
 */

/**
 * This special Python method is implement to provide support for the pickle
 * module.
 */
INLINE_LINMATH PyObject *Extension<FLOATNAME(LMatrix4)>::
__reduce__(PyObject *self) const {
  // We should return at least a 2-tuple, (Class, (args)): the necessary class
  // object whose constructor we should call (e.g.  this), and the arguments
  // necessary to reconstruct this object.
  PyObject *this_class = PyObject_Type(self);
  if (this_class == nullptr) {
    return nullptr;
  }

  PyObject *result = Py_BuildValue("(O(ffffffffffffffff))", this_class,
    _this->_m(0, 0), _this->_m(0, 1), _this->_m(0, 2), _this->_m(0, 3),
    _this->_m(1, 0), _this->_m(1, 1), _this->_m(1, 2), _this->_m(1, 3),
    _this->_m(2, 0), _this->_m(2, 1), _this->_m(2, 2), _this->_m(2, 3),
    _this->_m(3, 0), _this->_m(3, 1), _this->_m(3, 2), _this->_m(3, 3));

  Py_DECREF(this_class);
  return result;
}

/**
 *
 */
INLINE_LINMATH FLOATNAME(LMatrix4) Extension<FLOATNAME(LMatrix4)>::
__rmul__(FLOATTYPE scalar) const {
  return *_this * scalar;
}

/**
 *
 */
INLINE_LINMATH std::string Extension<FLOATNAME(LMatrix4)>::
__repr__() const {
  char buf[32 * 17] = "LMatrix4";
  char *p = buf + strlen(buf);
  *(p++) = FLOATTOKEN;
  *(p++) = '(';
  FLOATTYPE_REPR(_this->_m(0, 0), p);
  p += strlen(p);

  for (int i = 1; i < 16; ++i) {
    *(p++) = ',';
    *(p++) = ' ';
    FLOATTYPE_REPR(_this->get_data()[i], p);
    p += strlen(p);
  }

  *(p++) = ')';
  *p = '\0';
  return std::string(buf, p - buf);
}
