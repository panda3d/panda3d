// Filename: filename_ext.cxx
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

#include "filename_ext.h"

#ifdef HAVE_PYTHON
////////////////////////////////////////////////////////////////////
//     Function: Extension<Filename>::__reduce__
//       Access: Published
//  Description: This special Python method is implement to provide
//               support for the pickle module.
////////////////////////////////////////////////////////////////////
PyObject *Extension<Filename>::
__reduce__(PyObject *self) const {
  // We should return at least a 2-tuple, (Class, (args)): the
  // necessary class object whose constructor we should call
  // (e.g. this), and the arguments necessary to reconstruct this
  // object.
  PyTypeObject *this_class = Py_TYPE(self);
  if (this_class == NULL) {
    return NULL;
  }

  PyObject *result = Py_BuildValue("(O(s))", this_class, _this->c_str());
  return result;
}

////////////////////////////////////////////////////////////////////
//     Function: Extension<Filename>::__repr__
//       Access: Published
//  Description: Returns a string representation of the filename that
//               communicates both its type and value.
////////////////////////////////////////////////////////////////////
PyObject *Extension<Filename>::
__repr__() const {
#if PY_MAJOR_VERSION >= 3
  // Python 3 case: return a unicode object.
  wstring filename = _this->get_fullpath_w();
  PyObject *str = PyUnicode_FromWideChar(filename.data(), (Py_ssize_t)filename.size());

#if PY_VERSION_HEX >= 0x03040000
  PyObject *result = PyUnicode_FromFormat("Filename(%R)", str);
#else
  static PyObject *format = PyUnicode_FromString("Filename(%r)");
  PyObject *result = PyUnicode_Format(format, str);
#endif

#else
  // Python 2 case: return a regular string.
  string filename = _this->get_fullpath();
  PyObject *str = PyString_FromStringAndSize(filename.data(), (Py_ssize_t)filename.size());
  static PyObject *format = PyString_FromString("Filename(%r)");
  PyObject *result = PyString_Format(format, str);
#endif

  Py_DECREF(str);
  return result;
}

////////////////////////////////////////////////////////////////////
//     Function: Extension<Filename>::scan_directory
//       Access: Published
//  Description: This variant on scan_directory returns a Python list
//               of strings on success, or None on failure.
////////////////////////////////////////////////////////////////////
PyObject *Extension<Filename>::
scan_directory() const {
  vector_string contents;
  if (!_this->scan_directory(contents)) {
    Py_INCREF(Py_None);
    return Py_None;
  }

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


