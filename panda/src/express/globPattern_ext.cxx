// Filename: globPattern_ext.cxx
// Created by:  rdb (17Sep14)
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

#include "globPattern_ext.h"

#ifdef HAVE_PYTHON

////////////////////////////////////////////////////////////////////
//     Function: Extension<GlobPattern>::match_files
//       Access: Published
//  Description: This variant on match_files returns a Python list
//               of strings.
////////////////////////////////////////////////////////////////////
PyObject *Extension<GlobPattern>::
match_files(const Filename &cwd) const {
  vector_string contents;
  _this->match_files(contents, cwd);

  PyObject *result = PyList_New(contents.size());
  for (size_t i = 0; i < contents.size(); ++i) {
    const string &filename = contents[i];
#if PY_MAJOR_VERSION >= 3
    // This function expects UTF-8.
    PyObject *str = PyUnicode_FromStringAndSize(filename.data(), filename.size());
#else
    PyObject *str = PyString_FromStringAndSize(filename.data(), filename.size());
#endif
    PyList_SET_ITEM(result, i, str);
  }

  return result;
}

#endif  // HAVE_PYTHON
