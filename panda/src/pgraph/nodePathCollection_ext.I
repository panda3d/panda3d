// Filename: nodePathCollection_ext.I
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

#ifndef CPPPARSER
#ifdef STDFLOAT_DOUBLE
IMPORT_THIS struct Dtool_PyTypedObject Dtool_LPoint3d;
#else
IMPORT_THIS struct Dtool_PyTypedObject Dtool_LPoint3f;
#endif
#endif

////////////////////////////////////////////////////////////////////
//     Function: Extension<NodePathCollection>::get_tight_bounds
//       Access: Published
//  Description: Returns the tight bounds as a 2-tuple of LPoint3
//               objects.  This is a convenience function for Python
//               users, among which the use of calc_tight_bounds
//               may be confusing.
//               Returns None if calc_tight_bounds returned false.
////////////////////////////////////////////////////////////////////
INLINE PyObject *Extension<NodePathCollection>::
get_tight_bounds() const {
  LPoint3 *min_point = new LPoint3;
  LPoint3 *max_point = new LPoint3;

  if (_this->calc_tight_bounds(*min_point, *max_point)) {
#ifdef STDFLOAT_DOUBLE
    PyObject *min_inst = DTool_CreatePyInstance((void*) min_point, Dtool_LPoint3d, true, false);
    PyObject *max_inst = DTool_CreatePyInstance((void*) max_point, Dtool_LPoint3d, true, false);
#else
    PyObject *min_inst = DTool_CreatePyInstance((void*) min_point, Dtool_LPoint3f, true, false);
    PyObject *max_inst = DTool_CreatePyInstance((void*) max_point, Dtool_LPoint3f, true, false);
#endif
    return Py_BuildValue("NN", min_inst, max_inst);

  } else {
    Py_INCREF(Py_None);
    return Py_None;
  }
}
