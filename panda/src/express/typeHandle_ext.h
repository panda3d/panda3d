// Filename: typeHandle_ext.h
// Created by:  rdb (17Sep14)
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

#ifndef TYPEHANDLE_EXT_H
#define TYPEHANDLE_EXT_H

#include "dtoolbase.h"

#ifdef HAVE_PYTHON

#include "extension.h"
#include "typeHandle.h"
#include "py_panda.h"

////////////////////////////////////////////////////////////////////
//       Class : Extension<TypeHandle>
// Description : This class defines the extension methods for
//               TypeHandle, which are called instead of
//               any C++ methods with the same prototype.
////////////////////////////////////////////////////////////////////
template<>
class Extension<TypeHandle> : public ExtensionBase<TypeHandle> {
public:
  static TypeHandle make(PyTypeObject *tp);
};

#endif  // HAVE_PYTHON

#endif  // TYPEHANDLE_EXT_H
