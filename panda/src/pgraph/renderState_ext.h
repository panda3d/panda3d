// Filename: renderState_ext.h
// Created by:  CFSworks (31Mar14)
//
////////////////////////////////////////////////////////////////////
//
// PANDA 3D SOFTWARE
// Copyright (c) Carnegie Mellon University.  All rights reserved.
//
// All use of this software is subject to the terms of the revised BSD
// license.  You should have received a copy of this license along
// with this source code in a file named "LICENSE."
//
////////////////////////////////////////////////////////////////////

#ifndef RENDERSTATE_EXT_H
#define RENDERSTATE_EXT_H

#include "dtoolbase.h"

#ifdef HAVE_PYTHON

#include "extension.h"
#include "renderState.h"
#include "py_panda.h"

////////////////////////////////////////////////////////////////////
//       Class : Extension<RenderState>
// Description : This class defines the extension methods for
//               RenderState, which are called instead of
//               any C++ methods with the same prototype.
////////////////////////////////////////////////////////////////////
template<>
class Extension<RenderState> : public ExtensionBase<RenderState> {
public:
  PyObject *get_composition_cache() const;
  PyObject *get_invert_composition_cache() const;
  static PyObject *get_states();
};

#endif  // HAVE_PYTHON

#endif  // RENDERSTATE_EXT_H
