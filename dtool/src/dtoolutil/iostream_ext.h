/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file iostream_ext.h
 * @author rdb
 * @date 2017-07-24
 */

#ifndef IOSTREAM_EXT_H
#define IOSTREAM_EXT_H

#include "dtoolbase.h"

#ifdef HAVE_PYTHON

#include "extension.h"
#include <iostream>
#include "py_panda.h"

/**
 * These classes define the extension methods for istream and ostream, which
 * are called instead of any C++ methods with the same prototype.
 *
 * These are designed to allow streams to be treated as file-like objects.
 */
template<>
class Extension<istream> : public ExtensionBase<istream> {
public:
  PyObject *read(Py_ssize_t size=-1);
  PyObject *read1(Py_ssize_t size=-1);
  PyObject *readall();
  std::streamsize readinto(PyObject *b);

  PyObject *readline(Py_ssize_t size=-1);
  PyObject *readlines(Py_ssize_t hint=-1);
  PyObject *__iter__(PyObject *self);
};

template<>
class Extension<ostream> : public ExtensionBase<ostream> {
public:
  void write(PyObject *b);
  void writelines(PyObject *lines);
};

#endif  // HAVE_PYTHON

#endif  // IOSTREAM_EXT_H
