// Filename: parameter_ext.cxx
// Created by:  rdb (28Jul15)
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

#include "parameter_ext.h"

#include "py_panda.h"

#ifdef HAVE_PYTHON
#ifndef CPPPARSER
IMPORT_THIS struct Dtool_PyTypedObject Dtool_TypedObject;
#endif
#endif

////////////////////////////////////////////////////////////////////
//     Function: Parameter::set_value
//       Access: Published
//  Description: Changes the value associated with this Parameter.
////////////////////////////////////////////////////////////////////
void Extension<Parameter>::
set_value(PyObject *value) {
  if (value == Py_None) {
    _this->clear();

  } else if (DtoolCanThisBeAPandaInstance(value)) {
    //

  } else if (value == Py_True) {
    _this->set_value(true);

  } else if (value == Py_False) {
    _this->set_value(false);

  } else if (PyFloat_CheckExact(value)) {
    _this->set_value(PyFloat_AS_DOUBLE(value));

#if PY_MAJOR_VERSION < 3
  } else if (PyInt_CheckExact(value)) {
    _this->set_value((int) PyInt_AS_LONG(value));
#endif

  } else if (PyLong_CheckExact(value)) {
    _this->set_value((int) PyLong_AsLong(value));

  } else {
    nassertv(false);
    // raise TypeError;
  }
}

////////////////////////////////////////////////////////////////////
//     Function: Parameter::get_value
//       Access: Published
//  Description: Returns the value associated with this Parameter.
////////////////////////////////////////////////////////////////////
PyObject *Extension<Parameter>::
get_value() const {
  switch (_this->do_get_type()) {
  case Parameter::T_pointer:
    {
      ReferenceCount *ptr = _this->do_get_ptr();

      if (ptr != (ReferenceCount *)NULL) {
        ptr->ref();

        TypedObject *obj = ptr->as_typed_object();
        nassertr(obj != NULL, NULL);
        //return DTool_CreatePyInstanceTyped(obj, true);
        return DTool_CreatePyInstanceTyped((void *)obj, Dtool_TypedObject, true,
                                           false, obj->get_type().get_index());
      }
      break;
    }

  case Parameter::T_bool:
    {
      PyObject *value = _this->_v._packed._bool ? Py_True : Py_False;
      Py_INCREF(value);
      return value;
    }

  case Parameter::T_int:
#if PY_MAJOR_VERSION >= 3
    return PyLong_FromLong(_this->_v._packed._int);
#else
    return PyInt_FromLong(_this->_v._packed._int);
#endif

  case Parameter::T_float:
    return PyFloat_FromDouble(_this->_v._packed._float);

  default:
    return PyFloat_FromDouble(_this->do_get_double());
  }

  Py_INCREF(Py_None);
  return Py_None;
}

