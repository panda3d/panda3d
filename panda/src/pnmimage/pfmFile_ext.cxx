/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file pfmFile_ext.cxx
 * @author rdb
 * @date 2014-02-26
 */

#include "pfmFile_ext.h"

#ifdef HAVE_PYTHON

#ifndef CPPPARSER
extern struct Dtool_PyTypedObject Dtool_LPoint2f;
extern struct Dtool_PyTypedObject Dtool_LPoint3f;
extern struct Dtool_PyTypedObject Dtool_LPoint4f;
#endif

/**
 * Returns a list of all of the points.
 */
PyObject *Extension<PfmFile>::
get_points() const {
  int num_points = _this->get_x_size() * _this->get_y_size();
  PyObject *list = PyList_New(num_points);
  const vector_float &table = _this->get_table();

  switch (_this->get_num_channels()) {
  case 1:
    for (int i = 0; i < num_points; ++i) {
      PyList_SET_ITEM(list, i, PyFloat_FromDouble(table[i]));
    }
    break;

  case 2:
    for (int i = 0; i < num_points; ++i) {
      LPoint2f *point = (LPoint2f *) &(table[i * 2]);
      PyObject *item = DTool_CreatePyInstance((void *)point, Dtool_LPoint2f, false, true);
      PyList_SET_ITEM(list, i, item);
    }
    break;

  case 3:
    for (int i = 0; i < num_points; ++i) {
      LPoint3f *point = (LPoint3f *) &(table[i * 3]);
      PyObject *item = DTool_CreatePyInstance((void *)point, Dtool_LPoint3f, false, true);
      PyList_SET_ITEM(list, i, item);
    }
    break;

  case 4:
    for (int i = 0; i < num_points; ++i) {
      LPoint4f *point = (LPoint4f *) &(table[i * 4]);
      PyObject *item = DTool_CreatePyInstance((void *)point, Dtool_LPoint4f, false, true);
      PyList_SET_ITEM(list, i, item);
    }
    break;

  default:
    Py_DECREF(list);
    Py_INCREF(Py_None);
    return Py_None;
  }

  return list;
}

/**
 * This is a very low-level function that returns a read-only multiview into
 * the internal table of floating-point numbers.  Use this method at your own
 * risk.
 */
int Extension<PfmFile>::
__getbuffer__(PyObject *self, Py_buffer *view, int flags) const {
#if PY_VERSION_HEX >= 0x02060000
  if ((flags & PyBUF_WRITABLE) == PyBUF_WRITABLE) {
      PyErr_SetString(PyExc_BufferError,
                      "Object is not writable.");
      return -1;
  }

  // Since we have absolutely no guarantees about the lifetime of this object
  // or the continued validity of the data pointer, we should arguably make a
  // copy of the data.  However, since the whole point of this method is to
  // provide fast access to the underlying data, perhaps we can trust the user
  // to handle the copy operation himself if he needs to.
  const vector_float &table = _this->get_table();
  int channels = _this->get_num_channels();
  int num_pixels = _this->get_x_size() * _this->get_y_size();

  if (self != nullptr) {
    Py_INCREF(self);
  }
  view->obj = self;
  view->buf = (void *) &(table[0]);
  view->len = 4 * table.size();
  view->readonly = 1;
  view->itemsize = 4;
  view->format = nullptr;
  if ((flags & PyBUF_FORMAT) == PyBUF_FORMAT) {
    view->format = (char *) "f";
  }
  view->ndim = 2;
  view->shape = nullptr;
  if ((flags & PyBUF_ND) == PyBUF_ND) {
    // If you're leaking and you know it, clap your hands!
    view->shape = new Py_ssize_t[2];
    view->shape[0] = num_pixels;
    view->shape[1] = channels;
  }
  view->strides = nullptr;
  view->suboffsets = nullptr;

  return 0;
#else
  return -1;
#endif
}

#endif  // HAVE_PYTHON
