/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file lvecBase3_ext_src.h
 * @author rdb
 * @date 2013-09-13
 */

////////////////////////////////////////////////////////////////////
//       Class : Extension<LVecBase3>
// Description : This class defines the extension methods for
//               LVecBase3, which are called instead of
//               any C++ methods with the same prototype.
////////////////////////////////////////////////////////////////////
template<>
class Extension<FLOATNAME(LVecBase3)> : public ExtensionBase<FLOATNAME(LVecBase3)> {
public:
  INLINE_LINMATH PyObject *__reduce__(PyObject *self) const;
  INLINE_LINMATH PyObject *__getattr__(PyObject *self, const string &attr_name) const;
  INLINE_LINMATH int __setattr__(PyObject *self, const string &attr_name, PyObject *assign);
  INLINE_LINMATH string __repr__() const;

  INLINE_LINMATH FLOATNAME(LVecBase3) __pow__(FLOATTYPE exponent) const;
  INLINE_LINMATH PyObject *__ipow__(PyObject *self, FLOATTYPE exponent);
};

#include "lvecBase3_ext_src.I"
