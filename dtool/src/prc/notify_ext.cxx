/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file notify_ext.cxx
 * @author rdb
 * @date 2022-12-09
 */

#include "notify_ext.h"

#ifdef HAVE_PYTHON

/**
 * Changes the ostream that all subsequent Notify messages will be written to.
 * If the previous ostream was set with delete_later = true, this will delete
 * the previous ostream.  If ostream_ptr is None, this resets the default to
 * cerr.
 */
void Extension<Notify>::
set_ostream_ptr(PyObject *ostream_ptr, bool delete_later) {
  extern struct Dtool_PyTypedObject Dtool_std_ostream;

  if (ostream_ptr == Py_None) {
    _this->set_ostream_ptr(nullptr, false);
    return;
  }

  std::ostream *ptr = (std::ostream *)DTOOL_Call_GetPointerThisClass(ostream_ptr, &Dtool_std_ostream, 1, "Notify.set_ostream_ptr", false, true);
  if (ptr == nullptr) {
    return;
  }

  // Since we now have a reference to this class on the C++ end, make sure
  // that the ostream isn't being destructed when its Python wrapper expires.
  // Note that this may cause a memory leak if delete_later is not set, but
  // since these pointers are usually set once for the rest of time, this is
  // considered less of a problem than having the Python object destroy the
  // object while C++ is still using it.  See GitHub #1371.
  ((Dtool_PyInstDef *)ostream_ptr)->_memory_rules = false;
  _this->set_ostream_ptr(ptr, delete_later);
}

#endif  // HAVE_PYTHON
