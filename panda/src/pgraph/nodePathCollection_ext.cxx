// Filename: nodePathCollection_ext.cxx
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

#include "nodePathCollection_ext.h"

#ifdef HAVE_PYTHON

#ifndef CPPPARSER
extern struct Dtool_PyTypedObject Dtool_NodePath;
#ifdef STDFLOAT_DOUBLE
extern struct Dtool_PyTypedObject Dtool_LPoint3d;
#else
extern struct Dtool_PyTypedObject Dtool_LPoint3f;
#endif
#endif

////////////////////////////////////////////////////////////////////
//     Function: NodePathCollection::__init__
//       Access: Published
//  Description: This special constructor accepts a Python list of
//               NodePaths.  Since this constructor accepts a generic
//               PyObject *, it should be the last constructor listed
//               in the class record.
////////////////////////////////////////////////////////////////////
void Extension<NodePathCollection>::
__init__(PyObject *self, PyObject *sequence) {
  PyObject *fast = PySequence_Fast(sequence, "NodePathCollection constructor requires a sequence");
  if (fast == NULL) {
    return;
  }

  Py_ssize_t size = PySequence_Fast_GET_SIZE(fast);
  _this->reserve(size);

  for (int i = 0; i < size; ++i) {
    PyObject *item = PySequence_Fast_GET_ITEM(fast, i);
    if (item == NULL) {
      return;
    }

    NodePath *path;
    DTOOL_Call_ExtractThisPointerForType(item, &Dtool_NodePath, (void **)&path);
    if (path == (NodePath *)NULL) {
      // Unable to add item--probably it wasn't of the appropriate type.
      ostringstream stream;
      stream << "Element " << i << " in sequence passed to NodePathCollection constructor is not a NodePath";
      string str = stream.str();
      PyErr_SetString(PyExc_TypeError, str.c_str());
      Py_DECREF(fast);
      return;
    } else {
      _this->add_path(*path);
    }
  }

  Py_DECREF(fast);
}

////////////////////////////////////////////////////////////////////
//     Function: NodePathCollection::__reduce__
//       Access: Published
//  Description: This special Python method is implement to provide
//               support for the pickle module.
////////////////////////////////////////////////////////////////////
PyObject *Extension<NodePathCollection>::
__reduce__(PyObject *self) const {
  // Here we will return a 4-tuple: (Class, (args), None, iterator),
  // where iterator is an iterator that will yield successive
  // NodePaths.

  // We should return at least a 2-tuple, (Class, (args)): the
  // necessary class object whose constructor we should call
  // (e.g. this), and the arguments necessary to reconstruct this
  // object.

  PyObject *this_class = PyObject_Type(self);
  if (this_class == NULL) {
    return NULL;
  }

  // Since a NodePathCollection is itself an iterator, we can simply
  // pass it as the fourth tuple component.
  PyObject *result = Py_BuildValue("(O()OO)", this_class, Py_None, self);
  Py_DECREF(this_class);
  return result;
}

////////////////////////////////////////////////////////////////////
//     Function: Extension<NodePathCollection>::get_tight_bounds
//       Access: Published
//  Description: Returns the tight bounds as a 2-tuple of LPoint3
//               objects.  This is a convenience function for Python
//               users, among which the use of calc_tight_bounds
//               may be confusing.
//               Returns None if calc_tight_bounds returned false.
////////////////////////////////////////////////////////////////////
PyObject *Extension<NodePathCollection>::
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

#endif  // HAVE_PYTHON
