/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file sparseArray_ext.h
 * @author rdb
 * @date 2020-03-21
 */

#ifndef SPARSEARRAY_EXT_H
#define SPARSEARRAY_EXT_H

#include "dtoolbase.h"

#ifdef HAVE_PYTHON

#include "extension.h"
#include "sparseArray.h"
#include "py_panda.h"

/**
 * This class defines the extension methods for SparseArray, which are called
 * instead of any C++ methods with the same prototype.
 */
template<>
class Extension<SparseArray> : public ExtensionBase<SparseArray> {
public:
  INLINE bool __bool__() const;

  PyObject *__getstate__() const;
  void __setstate__(PyObject *state);
};

#include "sparseArray_ext.I"

#endif  // HAVE_PYTHON

#endif  // SPARSEARRAY_EXT_H
