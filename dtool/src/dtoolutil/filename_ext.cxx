/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file filename_ext.cxx
 * @author rdb
 * @date 2014-09-17
 */

#include "filename_ext.h"

using std::string;
using std::wstring;

#ifdef HAVE_PYTHON

#ifndef CPPPARSER
extern Dtool_PyTypedObject Dtool_Filename;
#endif  // CPPPARSER

/**
 * Constructs a Filename object from a str, bytes object, or os.PathLike.
 */
void Extension<Filename>::
__init__(PyObject *path) {
  nassertv(path != nullptr);
  nassertv(_this != nullptr);

  Py_ssize_t length;

  if (PyUnicode_CheckExact(path)) {
    if (Filename::get_filesystem_encoding() == TextEncoder::E_utf8) {
      const char *data = PyUnicode_AsUTF8AndSize(path, &length);
      if (data != nullptr) {
        (*_this) = string(data, length);
      }
    } else {
      wchar_t *data;
      data = PyUnicode_AsWideCharString(path, &length);
      (*_this) = wstring(data, length);
      PyMem_Free(data);
    }
    return;
  }

  if (PyBytes_CheckExact(path)) {
    char *data;
    PyBytes_AsStringAndSize(path, &data, &length);
    (*_this) = string(data, length);
    return;
  }

  if (Py_IS_TYPE(path, Dtool_GetPyTypeObject(&Dtool_Filename))) {
    // Copy constructor.
    *_this = *(Filename *)DtoolInstance_VOID_PTR(path);
    return;
  }

  // It must be an os.PathLike object.  Check for an __fspath__ method.
  PyObject *fspath = PyObject_GetAttrString((PyObject *)Py_TYPE(path), "__fspath__");
  if (fspath == nullptr) {
    PyErr_Format(PyExc_TypeError, "expected str, bytes or os.PathLike object, not %s", Py_TYPE(path)->tp_name);
    return;
  }

  PyObject *path_str = PyObject_CallFunctionObjArgs(fspath, path, nullptr);
  Py_DECREF(fspath);
  if (path_str == nullptr) {
    return;
  }

  if (PyUnicode_CheckExact(path_str)) {
    if (Filename::get_filesystem_encoding() == TextEncoder::E_utf8) {
      const char *data = PyUnicode_AsUTF8AndSize(path_str, &length);
      if (data != nullptr) {
        (*_this) = Filename::from_os_specific(string(data, length));
      }
    } else {
      wchar_t *data;
      data = PyUnicode_AsWideCharString(path_str, &length);
      (*_this) = Filename::from_os_specific_w(wstring(data, length));
      PyMem_Free(data);
    }

  } else if (PyBytes_CheckExact(path_str)) {
    char *data;
    PyBytes_AsStringAndSize(path_str, &data, &length);
    (*_this) = Filename::from_os_specific(string(data, length));

  } else {
    PyErr_Format(PyExc_TypeError, "expected str or bytes object, not %s", Py_TYPE(path_str)->tp_name);
  }
  Py_DECREF(path_str);
}

/**
 * This special Python method is implement to provide support for the pickle
 * module.
 */
PyObject *Extension<Filename>::
__reduce__(PyObject *self) const {
  // We should return at least a 2-tuple, (Class, (args)): the necessary class
  // object whose constructor we should call (e.g.  this), and the arguments
  // necessary to reconstruct this object.
  PyTypeObject *this_class = Py_TYPE(self);
  if (this_class == nullptr) {
    return nullptr;
  }

  PyObject *result = Py_BuildValue("(O(s))", this_class, _this->c_str());
  return result;
}

/**
 * Returns a string representation of the filename that communicates both its
 * type and value.
 */
PyObject *Extension<Filename>::
__repr__() const {
  wstring filename = _this->get_fullpath_w();
  PyObject *str = PyUnicode_FromWideChar(filename.data(), (Py_ssize_t)filename.size());

  PyObject *result = PyUnicode_FromFormat("Filename(%R)", str);

  Py_DECREF(str);
  return result;
}

/**
 * Allows a Filename object to be passed to any Python function that accepts
 * an os.PathLike object.
 */
PyObject *Extension<Filename>::
__fspath__() const {
  wstring filename = _this->to_os_specific_w();
  return PyUnicode_FromWideChar(filename.data(), (Py_ssize_t)filename.size());
}

/**
 * This variant on scan_directory returns a Python list of strings on success,
 * or None on failure.
 */
PyObject *Extension<Filename>::
scan_directory() const {
  vector_string contents;
  if (!_this->scan_directory(contents)) {
    return Py_NewRef(Py_None);
  }

  PyObject *result = PyList_New(contents.size());
  for (size_t i = 0; i < contents.size(); ++i) {
    const string &filename = contents[i];
    // This function expects UTF-8.
    PyObject *str = PyUnicode_FromStringAndSize(filename.data(), filename.size());
    PyList_SET_ITEM(result, i, str);
  }

  return result;
}
#endif  // HAVE_PYTHON
