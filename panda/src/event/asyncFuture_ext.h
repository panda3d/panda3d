/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file asyncFuture_ext.h
 * @author rdb
 * @date 2017-10-29
 */

#ifndef ASYNCFUTURE_EXT_H
#define ASYNCFUTURE_EXT_H

#include "extension.h"
#include "py_panda.h"
#include "asyncFuture.h"

#ifdef HAVE_PYTHON

/**
 * Extension class for AsyncFuture
 */
template<>
class Extension<AsyncFuture> : public ExtensionBase<AsyncFuture> {
public:
  static PyObject *__await__(PyObject *self);
  static PyObject *__iter__(PyObject *self) { return __await__(self); }

  PyObject *result(PyObject *timeout = Py_None) const;

  PyObject *add_done_callback(PyObject *self, PyObject *fn);

  static PyObject *gather(PyObject *args);
};

#endif  // HAVE_PYTHON

#endif  // ASYNCFUTURE_EXT_H
