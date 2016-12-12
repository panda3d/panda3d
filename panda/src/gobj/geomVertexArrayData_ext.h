/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file geomVertexArrayData_ext.h
 * @author rdb
 * @date 2013-09-13
 */

#ifndef GEOMVERTEXARRAYDATA_EXT_H
#define GEOMVERTEXARRAYDATA_EXT_H

#include "dtoolbase.h"

#ifdef HAVE_PYTHON

#include "extension.h"
#include "geomVertexArrayData.h"
#include "py_panda.h"

/**
 * This class defines the extension methods for GeomVertexArrayData, which are
 * called instead of any C++ methods with the same prototype.
 */
template<>
class Extension<GeomVertexArrayData> : public ExtensionBase<GeomVertexArrayData> {
public:
  int __getbuffer__(PyObject *self, Py_buffer *view, int flags);
  int __getbuffer__(PyObject *self, Py_buffer *view, int flags) const;
  void __releasebuffer__(PyObject *self, Py_buffer *view) const;
};

/**
 * This class defines the extension methods for GeomVertexArrayDataHandle,
 * which are called instead of any C++ methods with the same prototype.
 */
template<>
class Extension<GeomVertexArrayDataHandle> : public ExtensionBase<GeomVertexArrayDataHandle> {
public:
  void copy_data_from(PyObject *buffer);
  void copy_subdata_from(size_t to_start, size_t to_size,
                         PyObject *buffer);
  void copy_subdata_from(size_t to_start, size_t to_size,
                         PyObject *buffer,
                         size_t from_start, size_t from_size);
};

#endif  // HAVE_PYTHON

#endif  // GEOMVERTEXARRAYDATA_EXT_H
