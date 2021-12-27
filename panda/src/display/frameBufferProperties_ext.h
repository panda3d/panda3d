/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file frameBufferProperties_ext.h
 * @author rdb
 * @date 2021-12-13
 */

#ifndef FRAMEBUFFERPROPERTIES_EXT_H
#define FRAMEBUFFERPROPERTIES_EXT_H

#include "dtoolbase.h"

#ifdef HAVE_PYTHON

#include "extension.h"
#include "frameBufferProperties.h"
#include "py_panda.h"

/**
 * This class defines the extension methods for FrameBufferProperties, which are
 * called instead of any C++ methods with the same prototype.
 */
template<>
class Extension<FrameBufferProperties> : public ExtensionBase<FrameBufferProperties> {
public:
  PyObject *__getstate__() const;
  void __setstate__(PyObject *self, PyObject *state);
};

#endif  // HAVE_PYTHON

#endif  // FRAMEBUFFERPROPERTIES_EXT_H
