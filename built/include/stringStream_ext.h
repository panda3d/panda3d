/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file stringStream_ext.h
 * @author rdb
 * @date 2015-08-06
 */

#ifndef STRINGSTREAM_EXT_H
#define STRINGSTREAM_EXT_H

#include "dtoolbase.h"

#ifdef HAVE_PYTHON

#include "extension.h"
#include "stringStream.h"
#include "py_panda.h"

/**
 * This class defines the extension methods for StringStream, which are called
 * instead of any C++ methods with the same prototype.
 */
template<>
class Extension<StringStream> : public ExtensionBase<StringStream> {
public:
  void __init__(PyObject *source);

  PyObject *get_data();
  void set_data(PyObject *data);
};

#endif  // HAVE_PYTHON

#endif  // STRINGSTREAM_EXT_H
