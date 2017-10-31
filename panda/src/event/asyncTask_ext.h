/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file asyncTask_ext.h
 * @author rdb
 * @date 2017-10-29
 */

#ifndef ASYNCTASK_EXT_H
#define ASYNCTASK_EXT_H

#include "extension.h"
#include "py_panda.h"
#include "modelLoadRequest.h"

#ifdef HAVE_PYTHON

/**
 * Extension class for AsyncTask
 */
template<>
class Extension<AsyncTask> : public ExtensionBase<AsyncTask> {
public:
  static PyObject *__await__(PyObject *self);
  static PyObject *__iter__(PyObject *self) { return __await__(self); }
};

#endif  // HAVE_PYTHON

#endif  // ASYNCTASK_EXT_H
