/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file graphicsPipeSelection_ext.cxx
 * @author rdb
 * @date 2021-12-10
 */

#include "graphicsPipeSelection_ext.h"

#ifdef HAVE_PYTHON

#include "pythonLoaderFileType.h"

extern struct Dtool_PyTypedObject Dtool_GraphicsPipeSelection;

/**
 * Implements pickle support.
 */
PyObject *Extension<GraphicsPipeSelection>::
__reduce__() const {
  PyObject *func = PyObject_GetAttrString((PyObject *)&Dtool_GraphicsPipeSelection, "get_global_ptr");
  return Py_BuildValue("N()", func);
}

#endif
