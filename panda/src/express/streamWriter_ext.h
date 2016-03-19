/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file streamWriter_ext.h
 * @author rdb
 * @date 2015-09-19
 */

#ifndef STREAMWRITER_EXT_H
#define STREAMWRITER_EXT_H

#include "dtoolbase.h"

#ifdef HAVE_PYTHON

#include "extension.h"
#include "streamWriter.h"
#include "py_panda.h"

/**
 * This class defines the extension methods for StreamWriter, which are called
 * instead of any C++ methods with the same prototype.
 */
template<>
class Extension<StreamWriter> : public ExtensionBase<StreamWriter> {
public:
  void append_data(PyObject *data);
};

#endif  // HAVE_PYTHON

#endif  // STREAMWriter_EXT_H
