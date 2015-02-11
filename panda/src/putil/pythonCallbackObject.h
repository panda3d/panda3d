// Filename: pythonCallbackObject.h
// Created by:  drose (13Mar09)
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

#ifndef PYTHONCALLBACKOBJECT_H
#define PYTHONCALLBACKOBJECT_H

#include "pandabase.h"

#ifdef HAVE_PYTHON

#include "py_panda.h"
#include "callbackObject.h"

////////////////////////////////////////////////////////////////////
//       Class : PythonCallbackObject
// Description : This is a specialization on CallbackObject to allow
//               a callback to directly call an arbitarary Python
//               function.  Powerful!  But use with caution.
////////////////////////////////////////////////////////////////////
class PythonCallbackObject : public CallbackObject {
PUBLISHED:
  PythonCallbackObject(PyObject *function = Py_None);
  virtual ~PythonCallbackObject();
  ALLOC_DELETED_CHAIN(PythonCallbackObject);

  void set_function(PyObject *function);
  PyObject *get_function();

public:
  virtual void do_callback(CallbackData *cbdata);

private:
  void do_python_callback(CallbackData *cbdata);

  PyObject *_function;

public:
  static TypeHandle get_class_type() {
    return _type_handle;
  }
  static void init_type() {
    CallbackObject::init_type();
    register_type(_type_handle, "PythonCallbackObject",
                  CallbackObject::get_class_type());
  }
  virtual TypeHandle get_type() const {
    return get_class_type();
  }
  virtual TypeHandle force_init_type() {init_type(); return get_class_type();}

private:
  static TypeHandle _type_handle;
};

#include "pythonCallbackObject.I"

#endif  // HAVE_PYTHON

#endif
