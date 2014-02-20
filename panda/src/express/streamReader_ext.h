// Filename: streamReader_ext.h
// Created by:  rdb (09Dec13)
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

#ifndef STREAMREADER_EXT_H
#define STREAMREADER_EXT_H

#include "dtoolbase.h"

#ifdef HAVE_PYTHON

#include "extension.h"
#include "streamReader.h"
#include "py_panda.h"

////////////////////////////////////////////////////////////////////
//       Class : Extension<StreamReader>
// Description : This class defines the extension methods for
//               StreamReader, which are called instead of
//               any C++ methods with the same prototype.
////////////////////////////////////////////////////////////////////
template<>
class Extension<StreamReader> : public ExtensionBase<StreamReader> {
public:
  BLOCKING PyObject *readlines();
};

#endif  // HAVE_PYTHON

#endif  // STREAMREADER_EXT_H
