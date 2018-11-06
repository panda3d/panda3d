/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file internalName_ext.h
 * @author rdb
 * @date 2014-09-28
 */

#ifndef INTERNALNAME_EXT_H
#define INTERNALNAME_EXT_H

#include "dtoolbase.h"

#ifdef HAVE_PYTHON

#include "extension.h"
#include "internalName.h"
#include "py_panda.h"

/**
 * This class defines the extension methods for InternalName, which are called
 * instead of any C++ methods with the same prototype.
 */
template<>
class Extension<InternalName> : public ExtensionBase<InternalName> {
public:
  static PT(InternalName) make(PyObject *str);
};

#endif  // HAVE_PYTHON

#endif  // INTERNALNAME_EXT_H
