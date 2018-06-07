/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file lvecBase4_ext_src.h
 * @author rdb
 * @date 2013-09-13
 */

/**
 * This class defines the extension methods for LVecBase4, which are called
 * instead of any C++ methods with the same prototype.
 */
template<>
class Extension<FLOATNAME(LVecBase4)> : public ExtensionBase<FLOATNAME(LVecBase4)> {
public:
  INLINE_LINMATH PyObject *__reduce__(PyObject *self) const;
  INLINE_LINMATH PyObject *__getattr__(PyObject *self, const std::string &attr_name) const;
  INLINE_LINMATH int __setattr__(PyObject *self, const std::string &attr_name, PyObject *assign);
  INLINE_LINMATH std::string __repr__() const;

  INLINE_LINMATH FLOATNAME(LVecBase4) __pow__(FLOATTYPE exponent) const;
  INLINE_LINMATH PyObject *__ipow__(PyObject *self, FLOATTYPE exponent);
};

#include "lvecBase4_ext_src.I"
