/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file lmatrix3_ext_src.h
 * @author rdb
 * @date 2013-09-12
 */

/**
 * This class defines the extension methods for LMatrix3, which are called
 * instead of any C++ methods with the same prototype.
 */
template<>
class Extension<FLOATNAME(LMatrix3)> : public ExtensionBase<FLOATNAME(LMatrix3)> {
public:
  INLINE_LINMATH PyObject *__reduce__(PyObject *self) const;
  INLINE_LINMATH std::string __repr__() const;
};

#include "lmatrix3_ext_src.I"
