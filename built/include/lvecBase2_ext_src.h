/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file lvecBase2_ext_src.h
 * @author rdb
 * @date 2013-09-13
 */

#ifdef HAVE_PYTHON

/**
 * This class defines the extension methods for LVecBase2, which are called
 * instead of any C++ methods with the same prototype.
 */
template<>
class Extension<FLOATNAME(LVecBase2)> : public ExtensionBase<FLOATNAME(LVecBase2)> {
public:
  INLINE_LINMATH PyObject *__reduce__(PyObject *self) const;
  INLINE_LINMATH PyObject *__getattr__(PyObject *self, const std::string &attr_name) const;
  INLINE_LINMATH int __setattr__(PyObject *self, const std::string &attr_name, PyObject *assign);
  INLINE_LINMATH std::string __repr__() const;

  INLINE_LINMATH PyObject *__rmul__(PyObject *self, FLOATTYPE scalar) const;

  INLINE_LINMATH PyObject *__floordiv__(PyObject *self, FLOATTYPE scalar) const;
  INLINE_LINMATH PyObject *__ifloordiv__(PyObject *self, FLOATTYPE scalar);

  INLINE_LINMATH PyObject *__pow__(PyObject *self, FLOATTYPE exponent) const;
  INLINE_LINMATH PyObject *__ipow__(PyObject *self, FLOATTYPE exponent);

  INLINE_LINMATH PyObject *__round__(PyObject *self) const;
  INLINE_LINMATH PyObject *__floor__(PyObject *self) const;
  INLINE_LINMATH PyObject *__ceil__(PyObject *self) const;

  INLINE_LINMATH int __getbuffer__(PyObject *self, Py_buffer *view, int flags) const;
};

#include "lvecBase2_ext_src.I"

#endif
