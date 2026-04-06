/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file bitArray_ext.h
 * @author rdb
 * @date 2020-03-21
 */

#ifndef BITARRAY_EXT_H
#define BITARRAY_EXT_H

#include "dtoolbase.h"

#ifdef HAVE_PYTHON

#include "extension.h"
#include "bitArray.h"
#include "py_panda.h"

/**
 * This class defines the extension methods for BitArray, which are called
 * instead of any C++ methods with the same prototype.
 */
template<>
class Extension<BitArray> : public ExtensionBase<BitArray> {
public:
  void __init__(PyObject *init_value);

  INLINE bool __bool__() const;

  PyObject *__getstate__() const;
  void __setstate__(PyObject *state);
};

#include "bitArray_ext.I"

#endif  // HAVE_PYTHON

#endif  // BITARRAY_EXT_H
