/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file bitMask_ext.h
 * @author rdb
 * @date 2020-03-22
 */

#ifndef BITMASK_EXT_H
#define BITMASK_EXT_H

#include "dtoolbase.h"

#ifdef HAVE_PYTHON

#include "extension.h"
#include "bitMask.h"
#include "py_panda.h"

/**
 * This class defines the extension methods for BitMask, which are called
 * instead of any C++ methods with the same prototype.
 */
template<class WType, int nbits>
class Extension<BitMask<WType, nbits> > : public ExtensionBase<BitMask<WType, nbits> > {
public:
  INLINE bool __bool__() const;
  INLINE PyObject *__int__() const;
  INLINE PyObject *__reduce__(PyObject *self) const;
};

#include "bitMask_ext.I"

#endif  // HAVE_PYTHON

#endif  // BITMASK_EXT_H
