/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file ramfile_ext.h
 * @author rdb
 * @date 2013-12-10
 */

#ifndef RAMFILE_EXT_H
#define RAMFILE_EXT_H

#include "dtoolbase.h"

#ifdef HAVE_PYTHON

#include "extension.h"
#include "ramfile.h"
#include "py_panda.h"

/**
 * This class defines the extension methods for Ramfile, which are called
 * instead of any C++ methods with the same prototype.
 */
template<>
class Extension<Ramfile> : public ExtensionBase<Ramfile> {
public:
  PyObject *read(size_t length);
  PyObject *readline();
  PyObject *readlines();

  PyObject *get_data() const;
};

#endif  // HAVE_PYTHON

#endif  // RAMFILE_EXT_H
