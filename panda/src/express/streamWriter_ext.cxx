/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file streamWriter_ext.cxx
 * @author rdb
 * @date 2015-09-19
 */

#include "streamWriter_ext.h"

#ifdef HAVE_PYTHON

/**
 * Appends some more raw data to the end of the StreamWriter.
 */
void Extension<StreamWriter>::
append_data(PyObject *data) {
  Py_buffer view;
  if (PyObject_GetBuffer(data, &view, PyBUF_CONTIG_RO) == -1) {
    PyErr_SetString(PyExc_TypeError,
                    "append_data() requires a contiguous buffer");
    return;
  }

  _this->append_data(view.buf, view.len);
  PyBuffer_Release(&view);
}

#endif
