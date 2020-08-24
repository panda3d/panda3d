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
    return PyBytes_FromStringAndSize((char *)&data[0], data.size());
  }
  return PyBytes_FromStringAndSize("", 0);
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

  PyErr_SetString(PyExc_TypeError,
                  "StringStream requires a bytes or buffer object");
}

#endif
