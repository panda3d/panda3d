// Filename: internalName_ext.h
// Created by:  rdb (28Sep14)
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

#ifndef INTERNALNAME_EXT_H
#define INTERNALNAME_EXT_H

#include "dtoolbase.h"

#ifdef HAVE_PYTHON

#include "extension.h"
#include "internalName.h"
#include "py_panda.h"

////////////////////////////////////////////////////////////////////
//       Class : Extension<InternalName>
// Description : This class defines the extension methods for
//               InternalName, which are called instead of
//               any C++ methods with the same prototype.
////////////////////////////////////////////////////////////////////
template<>
class Extension<InternalName> : public ExtensionBase<InternalName> {
public:
#if PY_MAJOR_VERSION >= 3
  static PT(InternalName) make(PyUnicodeObject *str);
#else
  static PT(InternalName) make(PyStringObject *str);
#endif
};

#endif  // HAVE_PYTHON

#endif  // INTERNALNAME_EXT_H
