/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file stringStream_ext.cxx
 * @author rdb
 * @date 2013-12-09
 */

#include "stringStream_ext.h"

#ifdef HAVE_PYTHON

/**
 *
 */
void Extension<StringStream>::
__init__(PyObject *source) {
  set_data(source);
}

/**
 * Returns the contents of the data stream as a string.
 */
PyObject *Extension<StringStream>::
get_data() {
  _this->flush();
  const vector_uchar &data = _this->_buf.get_data();
  if (!data.empty()) {
#if PY_MAJOR_VERSION >= 3
    return PyBytes_FromStringAndSize((char *)&data[0], data.size());
#else
    return PyString_FromStringAndSize((char *)&data[0], data.size());
#endif
  }
#if PY_MAJOR_VERSION >= 3
  return PyBytes_FromStringAndSize("", 0);
#else
  return PyString_FromStringAndSize("", 0);
#endif
}

/**
 * Replaces the contents of the data stream.  This implicitly reseeks to 0.
 */
void Extension<StringStream>::
set_data(PyObject *data) {
  _this->_buf.clear();

  if (data == nullptr) {
    return;
  }

#if PY_VERSION_HEX >= 0x02060000
  if (PyObject_CheckBuffer(data)) {
    Py_buffer view;
    if (PyObject_GetBuffer(data, &view, PyBUF_CONTIG_RO) == -1) {
      PyErr_SetString(PyExc_TypeError,
                      "StringStream requires a contiguous buffer");
      return;
    }
    _this->set_data((const unsigned char *)view.buf, view.len);
    PyBuffer_Release(&view);
    return;
  }
#endif

#if PY_MAJOR_VERSION < 3
  if (PyString_Check(data)) {
    char *buffer;
    Py_ssize_t length;
    if (PyString_AsStringAndSize(data, &buffer, &length) != -1) {
      _this->set_data((const unsigned char *)buffer, (size_t)length);
    }
    return;
  }
#endif

  PyErr_SetString(PyExc_TypeError,
                  "StringStream requires a bytes or buffer object");
}

#endif
