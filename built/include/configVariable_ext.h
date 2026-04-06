/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file configVariable_ext.h
 * @author rdb
 * @date 2021-12-10
 */

#ifndef CONFIGVARIABLE_EXT_H
#define CONFIGVARIABLE_EXT_H

#include "dtoolbase.h"

#ifdef HAVE_PYTHON

#include "extension.h"
#include "configVariable.h"
#include "py_panda.h"

/**
 * This class defines the extension methods for ConfigVariable, which are called
 * instead of any C++ methods with the same prototype.
 */
template<>
class Extension<ConfigVariable> : public ExtensionBase<ConfigVariable> {
public:
  PyObject *__reduce__(PyObject *self) const;
};

#endif  // HAVE_PYTHON

#endif  // CONFIGVARIABLE_EXT_H
