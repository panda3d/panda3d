// Filename: virtualFileSystem_ext.h
// Created by:  rdb (12Sep13)
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

#ifndef VIRTUALFILESYSTEM_EXT_H
#define VIRTUALFILESYSTEM_EXT_H

#include "dtoolbase.h"

#ifdef HAVE_PYTHON

#include "extension.h"
#include "virtualFileSystem.h"
#include "py_panda.h"

////////////////////////////////////////////////////////////////////
//       Class : Extension<VirtualFileSystem>
// Description : This class defines the extension methods for
//               VirtualFileSystem, which are called instead of
//               any C++ methods with the same prototype.
////////////////////////////////////////////////////////////////////
template<>
class Extension<VirtualFileSystem> : public ExtensionBase<VirtualFileSystem> {
public:
  PyObject *read_file(Filename const &filename, bool auto_unwrap) const;
  PyObject *write_file(Filename const &filename, PyObject *data, bool auto_wrap);
};

#endif  // HAVE_PYTHON

#endif  // VIRTUALFILESYSTEM_EXT_H

