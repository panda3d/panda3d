/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file paramPyObject.cxx
 * @author rdb
 * @date 2021-03-01
 */

#include "paramPyObject.h"

#ifdef HAVE_PYTHON

TypeHandle ParamPyObject::_type_handle;

/**
 * Decrements the reference count.
 */
ParamPyObject::
~ParamPyObject() {
#if defined(HAVE_THREADS) && !defined(SIMPLE_THREADS)
  PyGILState_STATE gstate;
  gstate = PyGILState_Ensure();
#endif

  Py_DECREF(_value);

#if defined(HAVE_THREADS) && !defined(SIMPLE_THREADS)
  PyGILState_Release(gstate);
#endif
}

/**
 *
 */
void ParamPyObject::
output(std::ostream &out) const {
  out << "<" << Py_TYPE(_value)->tp_name
      << " object at " << (void *)_value << ">";
}

#endif
