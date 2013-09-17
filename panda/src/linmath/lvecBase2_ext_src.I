// Filename: lvecBase2_ext_src.I
// Created by:  rdb (02Jan11)
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
IMPORT_THIS struct Dtool_PyTypedObject FLOATNAME(Dtool_LVecBase2);
IMPORT_THIS struct Dtool_PyTypedObject FLOATNAME(Dtool_LVecBase3);
IMPORT_THIS struct Dtool_PyTypedObject FLOATNAME(Dtool_LVecBase4);
#endif

////////////////////////////////////////////////////////////////////
//     Function: LVecBase2::__setitem__
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
INLINE_LINMATH void Extension<FLOATNAME(LVecBase2)>::
__setitem__(int i, FLOATTYPE v) {
  nassertv(i >= 0 && i < 2);
  _this->_v(i) = v;
}

////////////////////////////////////////////////////////////////////
//     Function: LVecBase2::python_repr
//       Access: Published
//  Description:
////////////////////////////////////////////////////////////////////
INLINE_LINMATH void Extension<FLOATNAME(LVecBase2)>::
python_repr(ostream &out, const string &class_name) const {
  out << class_name << "("
      << MAYBE_ZERO(_this->_v(0)) << ", "
      << MAYBE_ZERO(_this->_v(1)) << ")";
}

////////////////////////////////////////////////////////////////////
//     Function: LVecBase2::__reduce__
//       Access: Published
//  Description: This special Python method is implement to provide
//               support for the pickle module.
////////////////////////////////////////////////////////////////////
INLINE_LINMATH PyObject *Extension<FLOATNAME(LVecBase2)>::
__reduce__(PyObject *self) const {
  // We should return at least a 2-tuple, (Class, (args)): the
  // necessary class object whose constructor we should call
  // (e.g. this), and the arguments necessary to reconstruct this
  // object.
  PyObject *this_class = PyObject_Type(self);
  if (this_class == NULL) {
    return NULL;
  }

  PyObject *result = Py_BuildValue("(O(ff))", this_class,
                                   (*_this)[0], (*_this)[1]);
  Py_DECREF(this_class);
  return result;
}

////////////////////////////////////////////////////////////////////
//     Function: LVecBase2::__getattr__
//       Access: Published
//  Description: This is used to implement swizzle masks.
////////////////////////////////////////////////////////////////////
INLINE_LINMATH PyObject *Extension<FLOATNAME(LVecBase2)>::
__getattr__(const string &attr_name) const {
  // Validate the attribute name.
  for (string::const_iterator it = attr_name.begin(); it < attr_name.end(); it++) {
    if (*it != 'x' && *it != 'y') {
      return NULL;
    }
  }

  switch (attr_name.size()) {
    case 1:
      return PyFloat_FromDouble(_this->_v(attr_name[0] - 'x'));

    case 2: {
      FLOATNAME(LVecBase2) *vec = new FLOATNAME(LVecBase2);
      vec->_v(0) = _this->_v(attr_name[0] - 'x');
      vec->_v(1) = _this->_v(attr_name[1] - 'x');
      return DTool_CreatePyInstance((void *)vec, FLOATNAME(Dtool_LVecBase2), true, false);

    } case 3: {
      FLOATNAME(LVecBase3) *vec = new FLOATNAME(LVecBase3);
      vec->_v(0) = _this->_v(attr_name[0] - 'x');
      vec->_v(1) = _this->_v(attr_name[1] - 'x');
      vec->_v(2) = _this->_v(attr_name[2] - 'x');
      return DTool_CreatePyInstance((void *)vec, FLOATNAME(Dtool_LVecBase3), true, false);

    } case 4: {
      FLOATNAME(LVecBase4) *vec = new FLOATNAME(LVecBase4);
      vec->_v(0) = _this->_v(attr_name[0] - 'x');
      vec->_v(1) = _this->_v(attr_name[1] - 'x');
      vec->_v(2) = _this->_v(attr_name[2] - 'x');
      vec->_v(3) = _this->_v(attr_name[3] - 'x');
      return DTool_CreatePyInstance((void *)vec, FLOATNAME(Dtool_LVecBase4), true, false);
    }
  }

  return NULL;
}

////////////////////////////////////////////////////////////////////
//     Function: LVecBase2::__setattr__
//       Access: Published
//  Description: This is used to implement write masks.
////////////////////////////////////////////////////////////////////
INLINE_LINMATH int Extension<FLOATNAME(LVecBase2)>::
__setattr__(PyObject *self, const string &attr_name, PyObject *assign) {
#ifndef NDEBUG
  // Validate the attribute name.
  for (string::const_iterator it = attr_name.begin(); it < attr_name.end(); it++) {
    if (*it != 'x' && *it != 'y') {
      PyTypeObject *tp = self->ob_type;
      PyErr_Format(PyExc_AttributeError,
                   "'%.100s' object has no attribute '%.200s'",
                   tp->tp_name, attr_name.c_str());
      return -1;
    }
  }
#endif

  // It is a sequence, perhaps another vector?
  if (PySequence_Check(assign)) {

    // Whoosh.
    PyObject* fast = PySequence_Fast(assign, "");
    nassertr(fast != NULL, -1);

    // Let's be strict about size mismatches, to prevent user error.
    if (PySequence_Fast_GET_SIZE(fast) != (int)attr_name.size()) {
      PyErr_SetString(PyExc_ValueError, "length mismatch");
      Py_DECREF(fast);
      return -1;
    }

    // Get a pointer to the items, iterate over it and
    // perform our magic assignment.  Fast fast.  Oh yeah.
    PyObject** items = PySequence_Fast_ITEMS(fast);
    for (size_t i = 0; i < attr_name.size(); ++i) {

      PyObject* fl = PyNumber_Float(items[i]);
      if (fl == NULL) {
        // Oh darn.  Not when we've come this far.
        PyErr_SetString(PyExc_ValueError, "a sequence of floats is required");
        Py_DECREF(fast);
        return -1;
      }
      double value = PyFloat_AS_DOUBLE(fl);
      Py_DECREF(fl);

      _this->_v(attr_name[i] - 'x') = value;
    }

    Py_DECREF(fast);

  } else {
    // Maybe it's a single floating-point value.
    PyObject* fl = PyNumber_Float(assign);
    if (fl == NULL) {
      // It's not a floating-point value either?
      // Sheesh, I don't know what to do with it then.
      if (attr_name.size() == 1) {
        PyErr_SetString(PyExc_ValueError, "a float is required");
      } else {
        PyErr_Format(PyExc_ValueError, "'%.200s' object is not iterable",
          assign->ob_type->tp_name);
      }
      return -1;
    }
    double value = PyFloat_AS_DOUBLE(fl);
    Py_DECREF(fl);

    // Loop through the components in the attribute name,
    // and assign the floating-point value to every one of them.
    for (string::const_iterator it = attr_name.begin(); it < attr_name.end(); it++) {
      _this->_v((*it) - 'x') = value;
    }
  }

  return 0;
}
