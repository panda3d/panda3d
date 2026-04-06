/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file cInterval_ext.h
 * @author rdb
 * @date 2020-10-17
 */

#ifndef CINTERVAL_EXT_H
#define CINTERVAL_EXT_H

#include "dtoolbase.h"

#ifdef HAVE_PYTHON

#include "extension.h"
#include "cInterval.h"
#include "py_panda.h"

/**
 * This class defines the extension methods for CInterval, which are called
 * instead of any C++ methods with the same prototype.
 */
template<>
class Extension<CInterval> : public ExtensionBase<CInterval> {
public:
  PyObject *__await__(PyObject *self);
};

#endif  // HAVE_PYTHON

#endif  // CINTERVAL_EXT_H
