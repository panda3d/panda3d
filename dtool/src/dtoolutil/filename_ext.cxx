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
    wchar_t *data;
#if PY_VERSION_HEX >= 0x03020000
    data = PyUnicode_AsWideCharString(path, &length);
#else
    length = PyUnicode_GET_SIZE(path);
    data = (wchar_t *)alloca(sizeof(wchar_t) * (length + 1));
    PyUnicode_AsWideChar((PyUnicodeObject *)path, data, length);
#endif
    (*_this) = wstring(data, length);

#if PY_VERSION_HEX >= 0x03020000
    PyMem_Free(data);
#endif
    return;
  }

  if (PyBytes_CheckExact(path)) {
    char *data;
    PyBytes_AsStringAndSize(path, &data, &length);
    (*_this) = string(data, length);
    return;
  }

  if (Py_TYPE(path) == &Dtool_Filename._PyType) {
    // Copy constructor.
    *_this = *(Filename *)DtoolInstance_VOID_PTR(path);
    return;
  }

  PyObject *path_str;

#if PY_VERSION_HEX >= 0x03060000
  // It must be an os.PathLike object.  Check for an __fspath__ method.
  PyObject *fspath = PyObject_GetAttrString((PyObject *)Py_TYPE(path), "__fspath__");
  if (fspath == nullptr) {
    PyErr_Format(PyExc_TypeError, "expected str, bytes or os.PathLike object, not %s", Py_TYPE(path)->tp_name);
    return;
  }

  path_str = PyObject_CallFunctionObjArgs(fspath, path, nullptr);
  Py_DECREF(fspath);
#else
  // There is no standard path protocol before Python 3.6, but let's try and
  // support taking pathlib paths anyway.  We don't version check this to
  // allow people to use backports of the pathlib module.
  if (PyObject_HasAttrString(path, "_format_parsed_parts")) {
    path_str = PyObject_Str(path);
  } else {
#if PY_VERSION_HEX >= 0x03040000
    PyErr_Format(PyExc_TypeError, "expected str, bytes, Path or Filename object, not %s", Py_TYPE(path)->tp_name);
#elif PY_MAJOR_VERSION >= 3
    PyErr_Format(PyExc_TypeError, "expected str, bytes or Filename object, not %s", Py_TYPE(path)->tp_name);
#else
    PyErr_Format(PyExc_TypeError, "expected str or unicode object, not %s", Py_TYPE(path)->tp_name);
#endif
    return;
  }
#endif

  if (path_str == nullptr) {
    return;
  }

  if (PyUnicode_CheckExact(path_str)) {
    wchar_t *data;
#if PY_VERSION_HEX >= 0x03020000
    data = PyUnicode_AsWideCharString(path_str, &length);
#else
    length = PyUnicode_GET_SIZE(path_str);
    data = (wchar_t *)alloca(sizeof(wchar_t) * (length + 1));
    PyUnicode_AsWideChar((PyUnicodeObject *)path_str, data, length);
#endif
    (*_this) = Filename::from_os_specific_w(wstring(data, length));

#if PY_VERSION_HEX >= 0x03020000
    PyMem_Free(data);
#endif

  } else if (PyBytes_CheckExact(path_str)) {
    char *data;
    PyBytes_AsStringAndSize(path_str, &data, &length);
    (*_this) = Filename::from_os_specific(string(data, length));

  } else {
#if PY_MAJOR_VERSION >= 3
    PyErr_Format(PyExc_TypeError, "expected str or bytes object, not %s", Py_TYPE(path_str)->tp_name);
#else
    PyErr_Format(PyExc_TypeError, "expected str or unicode object, not %s", Py_TYPE(path_str)->tp_name);
#endif
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
