// Filename: geomVertexArrayData_ext.h
// Created by:  rdb (13Sep13)
//
////////////////////////////////////////////////////////////////////
//
// PANDA 3D SOFTWARE
// Copyright (c) Carnegie Mellon University.  All rights reserved.
//
// All use of this software is subject to the terms of the revised BSD
// license.  You should have received a copy of this license along
// with this source code in a file named "LICENSE."
//
////////////////////////////////////////////////////////////////////

#ifndef GEOMVERTEXARRAYDATA_EXT_H
#define GEOMVERTEXARRAYDATA_EXT_H

#include "dtoolbase.h"

#ifdef HAVE_PYTHON

#include "extension.h"
#include "geomVertexArrayData.h"
#include "py_panda.h"

////////////////////////////////////////////////////////////////////
//       Class : Extension<GeomVertexArrayData>
// Description : This class defines the extension methods for
//               GeomVertexArrayData, which are called instead of
//               any C++ methods with the same prototype.
////////////////////////////////////////////////////////////////////
template<>
class Extension<GeomVertexArrayData> : public ExtensionBase<GeomVertexArrayData> {
public:
#if PY_VERSION_HEX >= 0x02060000
  int __getbuffer__(PyObject *self, Py_buffer *view, int flags);
  int __getbuffer__(PyObject *self, Py_buffer *view, int flags) const;
  void __releasebuffer__(PyObject *self, Py_buffer *view) const;
#endif
};

////////////////////////////////////////////////////////////////////
//       Class : Extension<GeomVertexArrayDataHandle>
// Description : This class defines the extension methods for
//               GeomVertexArrayDataHandle, which are called instead
//               of any C++ methods with the same prototype.
////////////////////////////////////////////////////////////////////
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
