/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file windowProperties_ext.h
 * @author rdb
 * @date 2018-11-12
 */

#ifndef WINDOWPROPERTIES_EXT_H
#define WINDOWPROPERTIES_EXT_H

#include "dtoolbase.h"

#ifdef HAVE_PYTHON

#include "extension.h"
#include "windowProperties.h"
#include "py_panda.h"

/**
 * This class defines the extension methods for WindowProperties, which are
 * called instead of any C++ methods with the same prototype.
 */
template<>
class Extension<WindowProperties> : public ExtensionBase<WindowProperties> {
public:
  void __init__(PyObject *self, PyObject *args, PyObject *kwds);
};

#endif  // HAVE_PYTHON

#endif  // WINDOWPROPERTIES_EXT_H
