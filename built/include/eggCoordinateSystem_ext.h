/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file eggCoordinateSystem_ext.h
 * @author rdb
 * @date 2021-01-01
 */

#ifndef EGGCOORDINATESYSTEM_EXT_H
#define EGGCOORDINATESYSTEM_EXT_H

#include "dtoolbase.h"

#ifdef HAVE_PYTHON

#include "extension.h"
#include "eggCoordinateSystem.h"
#include "py_panda.h"

/**
 * This class defines the extension methods for EggCoordinateSystem, which are
 * called instead of any C++ methods with the same prototype.
 */
template<>
class Extension<EggCoordinateSystem> : public ExtensionBase<EggCoordinateSystem> {
public:
  PyObject *__reduce__() const;
};

#endif  // HAVE_PYTHON

#endif  // EGGCOORDINATESYSTEM_EXT_H
