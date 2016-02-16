/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file rocketRegion_ext.h
 * @author rdb
 * @date 2013-09-13
 */

#ifndef ROCKETREGION_EXT_H
#define ROCKETREGION_EXT_H

#include "dtoolbase.h"

#ifdef HAVE_PYTHON

#include "extension.h"
#include "rocketRegion.h"
#include "py_panda.h"

/**
 * This class defines the extension methods for GeomVertexArrayData, which are
 * called instead of any C++ methods with the same prototype.
 */
template<>
class Extension<RocketRegion> : public ExtensionBase<RocketRegion> {
public:
  PyObject *get_context() const;
};

#endif  // HAVE_PYTHON

#endif  // ROCKETREGION_EXT_H
