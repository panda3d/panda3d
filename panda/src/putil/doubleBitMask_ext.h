/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file doubleBitMask_ext.h
 * @author rdb
 * @date 2020-04-01
 */

#ifndef DOUBLEBITMASK_EXT_H
#define DOUBLEBITMASK_EXT_H

#include "dtoolbase.h"

#ifdef HAVE_PYTHON

#include "extension.h"
#include "doubleBitMask.h"
#include "py_panda.h"

#include "bitMask_ext.h"

/**
 * This class defines the extension methods for DoubleBitMask, which are called
 * instead of any C++ methods with the same prototype.
 */
template<class BMType>
class Extension<DoubleBitMask<BMType> > : public ExtensionBase<DoubleBitMask<BMType> > {
public:
  INLINE void __init__(PyObject *init_value);

  INLINE bool __bool__() const;
  INLINE PyObject *__int__() const;
  INLINE PyObject *__reduce__(PyObject *self) const;
};

#include "doubleBitMask_ext.I"

#endif  // HAVE_PYTHON

#endif  // DOUBLEBITMASK_EXT_H
