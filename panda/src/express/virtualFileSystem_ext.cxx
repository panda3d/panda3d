/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file virtualFileSystem_ext.cxx
 * @author rdb
 * @date 2013-09-12
 */

#include "virtualFileSystem_ext.h"
#include "vector_uchar.h"

#ifdef HAVE_PYTHON

/**
 * Convenience function; returns the entire contents of the indicated file as
 * a string (or as a bytes object, in Python 3).
 *
 * This variant on read_file() is implemented directly for Python, as a small
 * optimization, to avoid the double-construction of a string object that
 * would be otherwise required for the return value.
 */
PyObject *Extension<VirtualFileSystem>::
read_file(const Filename &filename, bool auto_unwrap) const {
  // Release the GIL while we do this potentially slow operation.
#if defined(HAVE_THREADS) && !defined(SIMPLE_THREADS)
  PyThreadState *_save;
  Py_UNBLOCK_THREADS
#endif

  vector_uchar pv;
  bool okflag = _this->read_file(filename, pv, auto_unwrap);

#if defined(HAVE_THREADS) && !defined(SIMPLE_THREADS)
  Py_BLOCK_THREADS
#endif

  if (!okflag) {
    return PyErr_Format(PyExc_IOError, "Failed to read file: '%s'", filename.c_str());
  }

#if PY_MAJOR_VERSION >= 3
  if (pv.empty()) {
    return PyBytes_FromStringAndSize("", 0);
  } else {
    return PyBytes_FromStringAndSize((const char *)&pv[0], pv.size());
  }
#else
  if (pv.empty()) {
    return PyString_FromStringAndSize("", 0);
  } else {
    return PyString_FromStringAndSize((const char *)&pv[0], pv.size());
  }
#endif
}

/**
 * Convenience function; writes the entire contents of the indicated file as a
 * string.
 *
 * This variant on write_file() is implemented directly for Python, as a small
 * optimization, to avoid the double-construction of a string object that
 * would be otherwise required.
 */
PyObject *Extension<VirtualFileSystem>::
write_file(const Filename &filename, PyObject *data, bool auto_wrap) {
  char *buffer;
  Py_ssize_t length;

#if PY_MAJOR_VERSION >= 3
  if (PyBytes_AsStringAndSize(data, &buffer, &length) == -1) {
    return nullptr;
  }
#else
  if (PyString_AsStringAndSize(data, &buffer, &length) == -1) {
    return nullptr;
  }
#endif

  bool result = _this->write_file(filename, (const unsigned char *)buffer, length, auto_wrap);
  return PyBool_FromLong(result);
}

#endif  // HAVE_PYTHON
