/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file geomVertexArrayData_ext.cxx
 * @author rdb
 * @date 2013-09-05
 */

#include "geomVertexArrayData_ext.h"

#ifdef HAVE_PYTHON

struct InternalBufferData {
  CPT(GeomVertexArrayDataHandle) _handle;
  Py_ssize_t _num_rows;
  Py_ssize_t _stride;
  std::string _format;
};

/**
 * This is used to implement the buffer protocol, in order to allow efficient
 * access to the array data through a Python multiview object.
 */
int Extension<GeomVertexArrayData>::
__getbuffer__(PyObject *self, Py_buffer *view, int flags) {
#if PY_VERSION_HEX >= 0x02060000
  PT(GeomVertexArrayDataHandle) handle = _this->modify_handle();
  CPT(GeomVertexArrayFormat) format = handle->get_array_format();

  int row_size;
  bool pad_fmt;

  if ((flags & PyBUF_STRIDES) == PyBUF_STRIDES) {
    // The consumer is fine with having a stride value.
    row_size = format->get_total_bytes();
    pad_fmt = false;
  } else {
    // The consumer expects a contiguous buffer.  Give the stride as row size,
    // and pad the format with extra bytes.
    row_size = format->get_stride();
    pad_fmt = true;
  }

  InternalBufferData *data = new InternalBufferData;
  data->_handle = handle;
  data->_num_rows = handle->get_num_rows();
  data->_stride = format->get_stride();
  data->_format = format->get_format_string(pad_fmt);

  view->internal = (void*) data;

  if (self != nullptr) {
    Py_INCREF(self);
  }
  view->obj = self;
  view->buf = (void*) handle->get_write_pointer();
  view->len = row_size * handle->get_num_rows();
  view->readonly = 0;
  view->itemsize = row_size;
  view->format = nullptr;
  if ((flags & PyBUF_FORMAT) == PyBUF_FORMAT) {
    view->format = (char*) data->_format.c_str();
  }
  view->ndim = 1;
  view->shape = nullptr;
  if ((flags & PyBUF_ND) == PyBUF_ND) {
    view->shape = &data->_num_rows;
  }
  view->strides = nullptr;
  if ((flags & PyBUF_STRIDES) == PyBUF_STRIDES) {
    view->strides = &data->_stride;
  }
  view->suboffsets = nullptr;

  return 0;
#else
  return -1;
#endif
}

/**
 * This is the const version of __getbuffer__, which does not support writing.
 */
int Extension<GeomVertexArrayData>::
__getbuffer__(PyObject *self, Py_buffer *view, int flags) const {
#if PY_VERSION_HEX >= 0x02060000
  if ((flags & PyBUF_WRITABLE) == PyBUF_WRITABLE) {
      PyErr_SetString(PyExc_BufferError,
                      "Object is not writable.");
      return -1;
  }

  CPT(GeomVertexArrayDataHandle) handle = _this->get_handle();
  CPT(GeomVertexArrayFormat) format = handle->get_array_format();

  int row_size;
  bool pad_fmt;

  if ((flags & PyBUF_STRIDES) == PyBUF_STRIDES) {
    // The consumer is fine with having a stride value.
    row_size = format->get_total_bytes();
    pad_fmt = false;
  } else {
    // The consumer expects a contiguous buffer.  Give the stride as row size,
    // and pad the format with extra bytes.
    row_size = format->get_stride();
    pad_fmt = true;
  }

  InternalBufferData *data = new InternalBufferData;
  data->_handle = handle;
  data->_num_rows = handle->get_num_rows();
  data->_stride = format->get_stride();
  data->_format = format->get_format_string(pad_fmt);

  view->internal = (void*) data;

  if (self != nullptr) {
    Py_INCREF(self);
  }
  view->obj = self;
  view->buf = (void*) handle->get_read_pointer(true);
  view->len = row_size * handle->get_num_rows();
  view->readonly = 1;
  view->itemsize = row_size;
  view->format = nullptr;
  if ((flags & PyBUF_FORMAT) == PyBUF_FORMAT) {
    view->format = (char*) data->_format.c_str();
  }
  view->ndim = 1;
  view->shape = nullptr;
  if ((flags & PyBUF_ND) == PyBUF_ND) {
    view->shape = &data->_num_rows;
  }
  view->strides = nullptr;
  if ((flags & PyBUF_STRIDES) == PyBUF_STRIDES) {
    view->strides = &data->_stride;
  }
  view->suboffsets = nullptr;

  return 0;
#else
  return -1;
#endif
}

/**
 * Releases the buffer allocated by __getbuffer__.
 */
void Extension<GeomVertexArrayData>::
__releasebuffer__(PyObject *self, Py_buffer *view) const {
#if PY_VERSION_HEX >= 0x02060000
  // Note: PyBuffer_Release automatically decrements view->obj.
  InternalBufferData *data;
  data = (InternalBufferData *) view->internal;
  if (data == nullptr) {
    return;
  }
  delete data;
  view->internal = nullptr;
#endif
}

/**
 * Copies all data from the given buffer object.  The array is rescaled as
 * necessary.
 */
void Extension<GeomVertexArrayDataHandle>::
copy_data_from(PyObject *buffer) {

#if PY_VERSION_HEX < 0x02060000
  PyErr_SetString(PyExc_TypeError, "buffer interface not supported before Python 2.6");

#else
  if (!PyObject_CheckBuffer(buffer)) {
    PyErr_SetString(PyExc_TypeError, "buffer object expected");
    return;
  }

  Py_buffer view;
  if (PyObject_GetBuffer(buffer, &view, PyBUF_CONTIG_RO) == -1) {
    PyErr_SetString(PyExc_TypeError, "contiguous buffer object expected");
    return;
  }

  _this->copy_data_from((const unsigned char *) view.buf, view.len);

  PyBuffer_Release(&view);
#endif
}

/**
 * Copies the entire data array from the buffer into a portion of the data
 * array of this object.  If to_size is not the size of the given buffer, the
 * size of this dat array is adjusted accordingly.
 */
void Extension<GeomVertexArrayDataHandle>::
copy_subdata_from(size_t to_start, size_t to_size, PyObject *buffer) {

#if PY_VERSION_HEX < 0x02060000
  PyErr_SetString(PyExc_TypeError, "buffer interface not supported before Python 2.6");

#else
  if (!PyObject_CheckBuffer(buffer)) {
    PyErr_SetString(PyExc_TypeError, "buffer object expected");
    return;
  }

  Py_buffer view;
  if (PyObject_GetBuffer(buffer, &view, PyBUF_CONTIG_RO) == -1) {
    PyErr_SetString(PyExc_TypeError, "contiguous buffer object expected");
    return;
  }

  _this->copy_subdata_from(to_start, to_size,
                           (const unsigned char *) view.buf,
                           0, (size_t) view.len);

  PyBuffer_Release(&view);
#endif
}

/**
 * Copies a portion of the data array from the buffer into a portion of the
 * data array of this object.  If to_size != from_size, the size of this data
 * array is adjusted accordingly.
 */
void Extension<GeomVertexArrayDataHandle>::
copy_subdata_from(size_t to_start, size_t to_size,
                  PyObject *buffer,
                  size_t from_start, size_t from_size) {

#if PY_VERSION_HEX < 0x02060000
  PyErr_SetString(PyExc_TypeError, "buffer interface not supported before Python 2.6");

#else
  if (!PyObject_CheckBuffer(buffer)) {
    PyErr_SetString(PyExc_TypeError, "buffer object expected");
    return;
  }

  Py_buffer view;
  if (PyObject_GetBuffer(buffer, &view, PyBUF_CONTIG_RO) == -1) {
    PyErr_SetString(PyExc_TypeError, "contiguous buffer object expected");
    return;
  }

  size_t from_buffer_orig_size = (size_t) view.len;
  from_start = std::min(from_start, from_buffer_orig_size);
  from_size = std::min(from_size, from_buffer_orig_size - from_start);

  _this->copy_subdata_from(to_start, to_size,
                           (const unsigned char *) view.buf,
                           from_start, from_size);

  PyBuffer_Release(&view);
#endif
}

#endif  // HAVE_PYTHON
