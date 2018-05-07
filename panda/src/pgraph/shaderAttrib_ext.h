/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file shaderAttrib_ext.h
 * @author rdb
 * @date 2017-10-08
 */

#ifndef SHADERATTRIB_EXT_H
#define SHADERATTRIB_EXT_H

#include "dtoolbase.h"

#ifdef HAVE_PYTHON

#include "extension.h"
#include "shaderAttrib.h"
#include "py_panda.h"

/**
 * This class defines the extension methods for ShaderAttrib, which are called
 * instead of any C++ methods with the same prototype.
 */
template<>
class Extension<ShaderAttrib> : public ExtensionBase<ShaderAttrib> {
public:
  CPT(RenderAttrib) set_shader_input(CPT_InternalName id, PyObject *value, int priority=0) const;
  CPT(RenderAttrib) set_shader_inputs(PyObject *args, PyObject *kwargs) const;
};

#endif  // HAVE_PYTHON

#endif  // SHADERATTRIB_EXT_H
