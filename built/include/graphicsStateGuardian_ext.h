/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file graphicsStateGuardian_ext.h
 * @author rdb
 * @date 2013-12-10
 */

#ifndef GRAPHICSSTATEGUARDIAN_EXT_H
#define GRAPHICSSTATEGUARDIAN_EXT_H

#include "dtoolbase.h"

#ifdef HAVE_PYTHON

#include "extension.h"
#include "graphicsStateGuardian.h"
#include "py_panda.h"

/**
 * This class defines the extension methods for Ramfile, which are called
 * instead of any C++ methods with the same prototype.
 */
template<>
class Extension<GraphicsStateGuardian> : public ExtensionBase<GraphicsStateGuardian> {
public:
  PyObject *get_prepared_textures() const;
};

#endif  // HAVE_PYTHON

#endif  // GRAPHICSSTATEGUARDIAN_EXT_H
