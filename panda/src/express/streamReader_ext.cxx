// Filename: streamReader_ext.cxx
// Created by:  rdb (09Dec13)
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

#include "streamReader_ext.h"

#ifdef HAVE_PYTHON

////////////////////////////////////////////////////////////////////
//     Function: StreamReader::readlines
//       Access: Published
//  Description: Reads all the lines at once and returns a list.
//               Also see the documentation for readline().
////////////////////////////////////////////////////////////////////
PyObject *Extension<StreamReader>::
readlines() {
  PyObject *lst = PyList_New(0);
  if (lst == NULL) {
    return NULL;
  }

  string line = _this->readline();
  while (!line.empty()) {
#if PY_MAJOR_VERSION >= 3
    PyObject *py_line = PyBytes_FromStringAndSize(line.data(), line.size());
#else
    PyObject *py_line = PyString_FromStringAndSize(line.data(), line.size());
#endif

    PyList_Append(lst, py_line);
    Py_DECREF(py_line);
  }

  return lst;
}

#endif

