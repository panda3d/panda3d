// Filename: callbackObject_ext.h
// Created by:  rdb (25Feb15)
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

#ifndef CALLBACKOBJECT_EXT_H
#define CALLBACKOBJECT_EXT_H

#include "dtoolbase.h"

#ifdef HAVE_PYTHON

#include "extension.h"
#include "pythonCallbackObject.h"
#include "pointerTo.h"
#include "py_panda.h"

////////////////////////////////////////////////////////////////////
//       Class : Extension<CallbackObject>
// Description : This class defines the extension methods for
//               CallbackObject, which are called instead of
//               any C++ methods with the same prototype.
//
//               This just defines a static constructor, which makes
//               it possible for Interrogate to automatically accept
//               a Python function wherever a CallbackObject is
//               accepted.
////////////////////////////////////////////////////////////////////
template<>
class Extension<CallbackObject> : public ExtensionBase<CallbackObject> {
public:
  INLINE static PT(CallbackObject) make(PyObject *function);
};

#include "callbackObject_ext.I"

#endif  // HAVE_PYTHON

#endif  // CALLBACKOBJECT_EXT_H
