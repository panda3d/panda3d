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

  return PyBytes_FromStringAndSize((char *)data, length);
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
  return PyBytes_FromStringAndSize(line.data(), line.size());
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
    PyObject *py_line = PyBytes_FromStringAndSize(line.data(), line.size());

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
  return PyBytes_FromStringAndSize(_this->_data.data(), _this->_data.size());
}

/**
 *
 */
PyObject *Extension<Ramfile>::
__getstate__() const {
  PyObject *state = PyTuple_New(2);
  PyTuple_SET_ITEM(state, 0, get_data());
  PyTuple_SET_ITEM(state, 1, PyLong_FromSize_t(_this->tell()));
  return state;
}

/**
 *
 */
void Extension<Ramfile>::
__setstate__(PyObject *state) {
  char *str;
  Py_ssize_t len;
  if (PyBytes_AsStringAndSize(PyTuple_GET_ITEM(state, 0), &str, &len) >= 0) {
    _this->_data = std::string(str, len);
  }
  _this->seek(PyLong_AsSize_t(PyTuple_GET_ITEM(state, 1)));
}

#endif
