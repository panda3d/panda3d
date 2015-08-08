// Filename: stringStream_ext.cxx
// Created by:  rdb (09Dec13)
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

#include "stringStream_ext.h"

#ifdef HAVE_PYTHON

////////////////////////////////////////////////////////////////////
//     Function: StringStream::__init__
//       Access: Published
//  Description:
////////////////////////////////////////////////////////////////////
void Extension<StringStream>::
__init__(PyObject *source) {
  set_data(source);
}

////////////////////////////////////////////////////////////////////
//     Function: StringStream::get_data
//       Access: Published
//  Description: Returns the contents of the data stream as a string.
////////////////////////////////////////////////////////////////////
PyObject *Extension<StringStream>::
get_data() {
  _this->flush();
  const pvector<unsigned char> &data = _this->_buf.get_data();
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

////////////////////////////////////////////////////////////////////
//     Function: StringStream::set_data
//       Access: Published
//  Description: Replaces the contents of the data stream.  This
//               implicitly reseeks to 0.
////////////////////////////////////////////////////////////////////
void Extension<StringStream>::
set_data(PyObject *data) {
  _this->_buf.clear();

  if (data == NULL) {
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
    pvector<unsigned char> pv;
    pv.insert(pv.end(), (const unsigned char *)view.buf, (const unsigned char *)view.buf + view.len);
    _this->_buf.swap_data(pv);
    PyBuffer_Release(&view);
    return;
  }
#endif

#if PY_MAJOR_VERSION < 3
  if (PyString_Check(data)) {
    char *buffer;
    Py_ssize_t length;
    if (PyString_AsStringAndSize(data, &buffer, &length) != -1) {
      pvector<unsigned char> pv;
      pv.insert(pv.end(), (const unsigned char *)buffer, (const unsigned char *)buffer + length);
      _this->_buf.swap_data(pv);
    }
    return;
  }
#endif

  PyErr_SetString(PyExc_TypeError,
                  "StringStream requires a bytes or buffer object");
}

#endif
