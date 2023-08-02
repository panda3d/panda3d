/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file typedWritable_ext.h
 * @author rdb
 * @date 2013-12-10
 */

#ifndef TYPEDWRITABLE_EXT_H
#define TYPEDWRITABLE_EXT_H

#include "dtoolbase.h"

#ifdef HAVE_PYTHON

#include "extension.h"
#include "typedWritable.h"
#include "py_panda.h"

/**
 * This class defines the extension methods for StreamReader, which are called
 * instead of any C++ methods with the same prototype.
 */
template<>
class Extension<TypedWritable> : public ExtensionBase<TypedWritable> {
public:
  static PyObject *__new__(PyTypeObject *cls);

  PyObject *__reduce__(PyObject *self) const;
  PyObject *__reduce_persist__(PyObject *self, PyObject *pickler) const;

  static PyObject *find_global_decode(PyObject *this_class, const char *func_name);
};

BEGIN_PUBLISH
PyObject *py_decode_TypedWritable_from_bam_stream(PyObject *this_class, const vector_uchar &data);
PyObject *py_decode_TypedWritable_from_bam_stream_persist(PyObject *unpickler, PyObject *this_class, const vector_uchar &data);
END_PUBLISH

#endif  // HAVE_PYTHON

#endif  // TYPEDWRITABLE_EXT_H
