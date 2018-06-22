/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file ramfile_ext.cxx
 * @author rdb
 * @date 2013-12-10
 */

#include "ramfile_ext.h"

#ifdef HAVE_PYTHON

/**
 * Extracts the indicated number of bytes in the stream and returns them as a
 * string (or bytes, in Python 3).  Returns empty string at end-of-file.
 */
PyObject *Extension<Ramfile>::
read(size_t length) {
  size_t data_length = _this->get_data_size();
  const char *data = _this->_data.data() + _this->_pos;
  length = std::min(length, data_length - _this->_pos);
  _this->_pos = std::min(_this->_pos + length, data_length);

#if PY_MAJOR_VERSION >= 3
  return PyBytes_FromStringAndSize((char *)data, length);
#else
  return PyString_FromStringAndSize((char *)data, length);
#endif
}

/**
 * Assumes the stream represents a text file, and extracts one line up to and
 * including the trailing newline character.  Returns empty string when the
 * end of file is reached.
 *
 * The interface here is intentionally designed to be similar to that for
 * Python's File.readline() function.
 */
PyObject *Extension<Ramfile>::
readline() {
  std::string line = _this->readline();
#if PY_MAJOR_VERSION >= 3
  return PyBytes_FromStringAndSize(line.data(), line.size());
#else
  return PyString_FromStringAndSize(line.data(), line.size());
#endif
}

/**
 * Reads all the lines at once and returns a list.  Also see the documentation
 * for readline().
 */
PyObject *Extension<Ramfile>::
readlines() {
  PyObject *lst = PyList_New(0);
  if (lst == nullptr) {
    return nullptr;
  }

  std::string line = _this->readline();
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

/**
 * Returns the entire buffer contents as a string, regardless of the current
 * data pointer.
 */
PyObject *Extension<Ramfile>::
get_data() const {
#if PY_MAJOR_VERSION >= 3
  return PyBytes_FromStringAndSize(_this->_data.data(), _this->_data.size());
#else
  return PyString_FromStringAndSize(_this->_data.data(), _this->_data.size());
#endif
}

#endif
