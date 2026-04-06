/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file transformState_ext.h
 * @author CFSworks
 * @date 2014-03-31
 */

#ifndef TRANSFORMSTATE_EXT_H
#define TRANSFORMSTATE_EXT_H

#include "dtoolbase.h"

#ifdef HAVE_PYTHON

#include "extension.h"
#include "transformState.h"
#include "py_panda.h"

/**
 * This class defines the extension methods for TransformState, which are
 * called instead of any C++ methods with the same prototype.
 */
template<>
class Extension<TransformState> : public ExtensionBase<TransformState> {
public:
  PyObject *get_composition_cache() const;
  PyObject *get_invert_composition_cache() const;
  static PyObject *get_states();
  static PyObject *get_unused_states();
};

#endif  // HAVE_PYTHON

#endif  // TRANSFORMSTATE_EXT_H
