/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file streamReader_ext.cxx
 * @author rdb
 * @date 2013-12-09
 */

#include "streamReader_ext.h"

#ifdef HAVE_PYTHON

/**
 * Extracts the indicated number of bytes in the stream and returns them as a
 * string (or bytes, in Python 3).  Returns empty string at end-of-file.
 */
PyObject *Extension<StreamReader>::
extract_bytes(size_t size) {
  std::istream *in = _this->get_istream();
  if (in->eof() || in->fail() || size == 0) {
    return PyBytes_FromStringAndSize(nullptr, 0);
  }

  PyObject *bytes = PyBytes_FromStringAndSize(nullptr, size);
  in->read(PyBytes_AS_STRING(bytes), size);
  size_t read_bytes = in->gcount();

  if (read_bytes == size || _PyBytes_Resize(&bytes, read_bytes) == 0) {
    return bytes;
  } else {
    return nullptr;
  }
}

/**
 * Assumes the stream represents a text file, and extracts one line up to and
 * including the trailing newline character.  Returns empty string when the
 * end of file is reached.
 *
 * The interface here is intentionally designed to be similar to that for
 * Python's File.readline() function.
 */
PyObject *Extension<StreamReader>::
readline() {
  std::istream *in = _this->get_istream();

  std::string line;
  int ch = in->get();
  while (ch != EOF && !in->fail()) {
    line += ch;
    if (ch == '\n' || in->eof()) {
      // Here's the newline character.
      break;
    }
    ch = in->get();
  }

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
PyObject *Extension<StreamReader>::
readlines() {
  PyObject *lst = PyList_New(0);
  if (lst == nullptr) {
    return nullptr;
  }

  PyObject *py_line = readline();

#if PY_MAJOR_VERSION >= 3
  while (PyBytes_GET_SIZE(py_line) > 0) {
#else
  while (PyString_GET_SIZE(py_line) > 0) {
#endif
    PyList_Append(lst, py_line);
    Py_DECREF(py_line);

    py_line = readline();
  }

  return lst;
}

#endif
