// Filename: pfmFile_ext.h
// Created by:  rdb (26Feb14)
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

#ifndef PFMFILE_EXT_H
#define PFMFILE_EXT_H

#include "dtoolbase.h"

#ifdef HAVE_PYTHON

#include "extension.h"
#include "pfmFile.h"
#include "py_panda.h"

////////////////////////////////////////////////////////////////////
//       Class : Extension<PfmFile>
// Description : This class defines the extension methods for
//               PfmFile, which are called instead of
//               any C++ methods with the same prototype.
////////////////////////////////////////////////////////////////////
template<>
class Extension<PfmFile> : public ExtensionBase<PfmFile> {
public:
  PyObject *get_points() const;

#if PY_VERSION_HEX >= 0x02060000
  int __getbuffer__(PyObject *self, Py_buffer *view, int flags) const;
#endif
};

#endif  // HAVE_PYTHON

#endif  // PFMFILE_EXT_H
