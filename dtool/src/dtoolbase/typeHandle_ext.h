/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file typeHandle_ext.h
 * @author rdb
 * @date 2014-09-17
 */

#ifndef TYPEHANDLE_EXT_H
#define TYPEHANDLE_EXT_H

#include "dtoolbase.h"

#ifdef HAVE_PYTHON

#include "extension.h"
#include "typeHandle.h"
#include "py_panda.h"

/**
 * This class defines the extension methods for TypeHandle, which are called
 * instead of any C++ methods with the same prototype.
 */
template<>
class Extension<TypeHandle> : public ExtensionBase<TypeHandle> {
public:
  static TypeHandle make(PyTypeObject *tp);
};

#endif  // HAVE_PYTHON

#endif  // TYPEHANDLE_EXT_H
