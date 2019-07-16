/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file pmutex_ext.h
 * @author rdb
 * @date 2019-05-12
 */

#ifndef PMUTEX_EXT_H
#define PMUTEX_EXT_H

#include "dtoolbase.h"

#ifdef HAVE_PYTHON

#include "extension.h"
#include "pmutex.h"
#include "py_panda.h"

/**
 * This class defines the extension methods for Mutex, which are called
 * instead of any C++ methods with the same prototype.
 */
template<>
class Extension<Mutex> : public ExtensionBase<Mutex> {
public:
  INLINE bool acquire(bool blocking) const;
  INLINE bool __enter__();
  INLINE void __exit__(PyObject *, PyObject *, PyObject *);
};

#include "pmutex_ext.I"

#endif  // HAVE_PYTHON

#endif  // PMUTEX_EXT_H
