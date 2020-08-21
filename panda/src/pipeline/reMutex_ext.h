/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file remutex_ext.h
 * @author rdb
 * @date 2019-05-12
 */

#ifndef REMUTEX_EXT_H
#define REMUTEX_EXT_H

#include "dtoolbase.h"

#ifdef HAVE_PYTHON

#include "extension.h"
#include "reMutex.h"
#include "py_panda.h"

/**
 * This class defines the extension methods for ReMutex, which are called
 * instead of any C++ methods with the same prototype.
 */
template<>
class Extension<ReMutex> : public ExtensionBase<ReMutex> {
public:
  INLINE bool acquire(bool blocking) const;
  INLINE bool __enter__();
  INLINE void __exit__(PyObject *, PyObject *, PyObject *);
};

#include "reMutex_ext.I"

#endif  // HAVE_PYTHON

#endif  // REMUTEX_EXT_H
