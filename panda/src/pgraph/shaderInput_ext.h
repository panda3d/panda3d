/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file shaderInput_ext.h
 * @author rdb
 * @date 2017-10-06
 */

#ifndef SHADERINPUT_EXT_H
#define SHADERINPUT_EXT_H

#include "dtoolbase.h"

#ifdef HAVE_PYTHON

#include "extension.h"
#include "shaderInput.h"
#include "py_panda.h"

/**
 * This class defines the extension methods for NodePath, which are called
 * instead of any C++ methods with the same prototype.
 */
template<>
class Extension<ShaderInput> : public ExtensionBase<ShaderInput> {
public:
  void __init__(CPT_InternalName name, PyObject *value, int priority=0);
};

#endif  // HAVE_PYTHON

#endif  // SHADERINPUT_EXT_H
