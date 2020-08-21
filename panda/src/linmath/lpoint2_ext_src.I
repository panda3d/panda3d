/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file lpoint2_ext_src.I
 * @author rdb
 * @date 2011-01-02
 */

/**
 *
 */
INLINE_LINMATH std::string Extension<FLOATNAME(LPoint2)>::
__repr__() const {
  std::ostringstream out;
  out << "LPoint2" << FLOATTOKEN << "("
      << MAYBE_ZERO(_this->_v(0)) << ", "
      << MAYBE_ZERO(_this->_v(1)) << ")";
  return out.str();
}

/**
 * This is used to implement swizzle masks.
 */
INLINE_LINMATH PyObject *Extension<FLOATNAME(LPoint2)>::
__getattr__(PyObject *self, const std::string &attr_name) const {
#ifndef CPPPARSER
  extern struct Dtool_PyTypedObject FLOATNAME(Dtool_LPoint2);
  extern struct Dtool_PyTypedObject FLOATNAME(Dtool_LPoint3);
  extern struct Dtool_PyTypedObject FLOATNAME(Dtool_LPoint4);
#endif

  // Validate the attribute name.
  for (std::string::const_iterator it = attr_name.begin(); it < attr_name.end(); it++) {
    if (*it != 'x' && *it != 'y') {
      return Dtool_Raise_AttributeError(self, attr_name.c_str());
    }
  }

  switch (attr_name.size()) {
    case 1:
      return Dtool_WrapValue(_this->_v(attr_name[0] - 'x'));

    case 2: {
      FLOATNAME(LPoint2) *vec = new FLOATNAME(LPoint2);
      vec->_v(0) = _this->_v(attr_name[0] - 'x');
      vec->_v(1) = _this->_v(attr_name[1] - 'x');
      return DTool_CreatePyInstance((void *)vec, FLOATNAME(Dtool_LPoint2), true, false);

    } case 3: {
      FLOATNAME(LPoint3) *vec = new FLOATNAME(LPoint3);
      vec->_v(0) = _this->_v(attr_name[0] - 'x');
      vec->_v(1) = _this->_v(attr_name[1] - 'x');
      vec->_v(2) = _this->_v(attr_name[2] - 'x');
      return DTool_CreatePyInstance((void *)vec, FLOATNAME(Dtool_LPoint3), true, false);

    } case 4: {
      FLOATNAME(LPoint4) *vec = new FLOATNAME(LPoint4);
      vec->_v(0) = _this->_v(attr_name[0] - 'x');
      vec->_v(1) = _this->_v(attr_name[1] - 'x');
      vec->_v(2) = _this->_v(attr_name[2] - 'x');
      vec->_v(3) = _this->_v(attr_name[3] - 'x');
      return DTool_CreatePyInstance((void *)vec, FLOATNAME(Dtool_LPoint4), true, false);
    }
  }

  return Dtool_Raise_AttributeError(self, attr_name.c_str());
}

/**
 * This is used to implement write masks.
 */
INLINE_LINMATH int Extension<FLOATNAME(LPoint2)>::
__setattr__(PyObject *self, const std::string &attr_name, PyObject *assign) {
  // Upcall to LVecBase2.
  return invoke_extension<FLOATNAME(LVecBase2)>(_this).__setattr__(self, attr_name, assign);
}
