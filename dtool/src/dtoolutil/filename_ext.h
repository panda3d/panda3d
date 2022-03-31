/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file filename_ext.h
 * @author rdb
 * @date 2014-09-17
 */

#ifndef FILENAME_EXT_H
#define FILENAME_EXT_H

#include "dtoolbase.h"

#ifdef HAVE_PYTHON

#include "extension.h"
#include "filename.h"
#include "py_panda.h"

/**
 * This class defines the extension methods for Filename, which are called
 * instead of any C++ methods with the same prototype.
 */
template<>
class Extension<Filename> : public ExtensionBase<Filename> {
public:
  void __init__(PyObject *path);

  PyObject *__reduce__(PyObject *self) const;
  PyObject *__repr__() const;
  PyObject *__fspath__() const;
  PyObject *scan_directory() const;
};

#endif  // HAVE_PYTHON

#endif  // FILENAME_EXT_H
