// Filename: geomVertexArrayData_ext.I
// Created by:  rdb (05Sep13)
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

#include "geomVertexArrayData_ext.h"

struct InternalBufferData {
  CPT(GeomVertexArrayDataHandle) _handle;
  Py_ssize_t _num_rows;
  Py_ssize_t _stride;
  string _format;
};

////////////////////////////////////////////////////////////////////
//     Function: GeomVertexArrayData::__getbuffer__
//       Access: Published
//  Description: This is used to implement the buffer protocol, in
//               order to allow efficient access to the array data
//               through a Python multiview object.
////////////////////////////////////////////////////////////////////
int Extension<GeomVertexArrayData>::
__getbuffer__(PyObject *self, Py_buffer *view, int flags) {

  PT(GeomVertexArrayDataHandle) handle = _this->modify_handle();
  CPT(GeomVertexArrayFormat) format = handle->get_array_format();

  int row_size;
  bool pad_fmt;

  if ((flags & PyBUF_STRIDES) == PyBUF_STRIDES) {
    // The consumer is fine with having a stride value.
    row_size = format->get_total_bytes();
    pad_fmt = false;
  } else {
    // The consumer expects a contiguous buffer.  Give the
    // stride as row size, and pad the format with extra bytes.
    row_size = format->get_stride();
    pad_fmt = true;
  }

  InternalBufferData *data = new InternalBufferData;
  data->_handle = handle;
  data->_num_rows = handle->get_num_rows();
  data->_stride = format->get_stride();
  data->_format = format->get_format_string(pad_fmt);

  view->internal = (void*) data;

  if (self != NULL) {
    Py_INCREF(self);
  }
  view->obj = self;
  view->buf = (void*) handle->get_write_pointer();
  view->len = row_size * handle->get_num_rows();
  view->readonly = 0;
  view->itemsize = row_size;
  view->format = NULL;
  if ((flags & PyBUF_FORMAT) == PyBUF_FORMAT) {
    view->format = (char*) data->_format.c_str();
  }
  view->ndim = 1;
  view->shape = NULL;
  if ((flags & PyBUF_ND) == PyBUF_ND) {
    view->shape = &data->_num_rows;
  }
  view->strides = NULL;
  if ((flags & PyBUF_STRIDES) == PyBUF_STRIDES) {
    view->strides = &data->_stride;
  }
  view->suboffsets = NULL;

  return 0;
}

////////////////////////////////////////////////////////////////////
//     Function: GeomVertexArrayData::__getbuffer__
//       Access: Published
//  Description: This is the const version of __getbuffer__, which
//               does not support writing.
////////////////////////////////////////////////////////////////////
int Extension<GeomVertexArrayData>::
__getbuffer__(PyObject *self, Py_buffer *view, int flags) const {

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
    // The consumer expects a contiguous buffer.  Give the
    // stride as row size, and pad the format with extra bytes.
    row_size = format->get_stride();
    pad_fmt = true;
  }

  InternalBufferData *data = new InternalBufferData;
  data->_handle = handle;
  data->_num_rows = handle->get_num_rows();
  data->_stride = format->get_stride();
  data->_format = format->get_format_string(pad_fmt);

  view->internal = (void*) data;

  if (self != NULL) {
    Py_INCREF(self);
  }
  view->obj = self;
  view->buf = (void*) handle->get_read_pointer(true);
  view->len = row_size * handle->get_num_rows();
  view->readonly = 1;
  view->itemsize = row_size;
  view->format = NULL;
  if ((flags & PyBUF_FORMAT) == PyBUF_FORMAT) {
    view->format = (char*) data->_format.c_str();
  }
  view->ndim = 1;
  view->shape = NULL;
  if ((flags & PyBUF_ND) == PyBUF_ND) {
    view->shape = &data->_num_rows;
  }
  view->strides = NULL;
  if ((flags & PyBUF_STRIDES) == PyBUF_STRIDES) {
    view->strides = &data->_stride;
  }
  view->suboffsets = NULL;

  return 0;
}

////////////////////////////////////////////////////////////////////
//     Function: GeomVertexArrayData::__releasebuffer__
//       Access: Published
//  Description: Releases the buffer allocated by __getbuffer__.
////////////////////////////////////////////////////////////////////
void Extension<GeomVertexArrayData>::
__releasebuffer__(PyObject *self, Py_buffer *view) const {
  // Note: PyBuffer_Release automatically decrements view->obj.

  InternalBufferData *data;
  data = (InternalBufferData *) view->internal;
  if (data == NULL) {
    return;
  }
  delete data;
  view->internal = NULL;
}
