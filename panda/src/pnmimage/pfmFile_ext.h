/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file pfmFile_ext.h
 * @author rdb
 * @date 2014-02-26
 */

#ifndef PFMFILE_EXT_H
#define PFMFILE_EXT_H

#include "dtoolbase.h"

#ifdef HAVE_PYTHON

#include "extension.h"
#include "pfmFile.h"
#include "py_panda.h"

/**
 * This class defines the extension methods for PfmFile, which are called
 * instead of any C++ methods with the same prototype.
 */
template<>
class Extension<PfmFile> : public ExtensionBase<PfmFile> {
public:
  PyObject *get_points() const;

  int __getbuffer__(PyObject *self, Py_buffer *view, int flags) const;
};

#endif  // HAVE_PYTHON

#endif  // PFMFILE_EXT_H
