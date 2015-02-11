// Filename: pythonCallbackObject.cxx
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

#include "pythonCallbackObject.h"

#ifdef HAVE_PYTHON

#include "py_panda.h"
#include "thread.h"
#include "callbackData.h"
#include "config_util.h"

TypeHandle PythonCallbackObject::_type_handle;

Configure(config_pythonCallbackObject);
ConfigureFn(config_pythonCallbackObject) {
  PythonCallbackObject::init_type();
}

#ifndef CPPPARSER
extern struct Dtool_PyTypedObject Dtool_TypedObject;
#endif

////////////////////////////////////////////////////////////////////
//     Function: PythonCallbackObject::Constructor
//       Access: Published
//  Description:
////////////////////////////////////////////////////////////////////
PythonCallbackObject::
PythonCallbackObject(PyObject *function) {
  _function = Py_None;
  Py_INCREF(_function);

  set_function(function);

#ifndef SIMPLE_THREADS
  // Ensure that the Python threading system is initialized and ready
  // to go.
#ifdef WITH_THREAD  // This symbol defined within Python.h

#if PY_VERSION_HEX >= 0x03020000
  Py_Initialize();
#endif

  PyEval_InitThreads();
#endif
#endif
}

////////////////////////////////////////////////////////////////////
//     Function: PythonCallbackObject::Destructor
//       Access: Published, Virtual
//  Description:
////////////////////////////////////////////////////////////////////
PythonCallbackObject::
~PythonCallbackObject() {
  Py_DECREF(_function);
}

////////////////////////////////////////////////////////////////////
//     Function: PythonCallbackObject::set_function
//       Access: Published
//  Description: Replaces the function that is called for the callback.
//               runs.  The parameter should be a Python callable
//               object.
////////////////////////////////////////////////////////////////////
void PythonCallbackObject::
set_function(PyObject *function) {
  Py_DECREF(_function);
  _function = function;
  Py_INCREF(_function);
  if (_function != Py_None && !PyCallable_Check(_function)) {
    nassert_raise("Invalid function passed to PythonCallbackObject");
  }
}

////////////////////////////////////////////////////////////////////
//     Function: PythonCallbackObject::get_function
//       Access: Published
//  Description: Returns the function that is called for the callback.
////////////////////////////////////////////////////////////////////
PyObject *PythonCallbackObject::
get_function() {
  Py_INCREF(_function);
  return _function;
}

////////////////////////////////////////////////////////////////////
//     Function: PythonCallbackObject::do_callback
//       Access: Public, Virtual
//  Description: This method called when the callback is triggered; it
//               *replaces* the original function.  To continue
//               performing the original function, you must call
//               cbdata->upcall() during the callback.
////////////////////////////////////////////////////////////////////
void PythonCallbackObject::
do_callback(CallbackData *cbdata) {
#if defined(HAVE_THREADS) && !defined(SIMPLE_THREADS)
  // Use PyGILState to protect this asynchronous call.
  PyGILState_STATE gstate;
  gstate = PyGILState_Ensure();
#endif

  do_python_callback(cbdata);

#if defined(HAVE_THREADS) && !defined(SIMPLE_THREADS)
  PyGILState_Release(gstate);
#endif
}

////////////////////////////////////////////////////////////////////
//     Function: PythonCallbackObject::do_python_callback
//       Access: Private
//  Description: The Python calls that implement do_callback().  This
//               function is separate so we can acquire the Python
//               interpretor lock while it runs.
////////////////////////////////////////////////////////////////////
void PythonCallbackObject::
do_python_callback(CallbackData *cbdata) {
  nassertv(cbdata != NULL);

  // Wrap the cbdata up in a Python object, then put it in a tuple,
  // for the argument list.
  PyObject *pycbdata =
    DTool_CreatePyInstanceTyped(cbdata, Dtool_TypedObject,
                                false, false, cbdata->get_type_index());
  PyObject *args = Py_BuildValue("(O)", pycbdata);
  Py_DECREF(pycbdata);

  PyObject *result =
    Thread::get_current_thread()->call_python_func(_function, args);
  Py_DECREF(args);

  if (result == (PyObject *)NULL) {
    util_cat.error()
      << "Exception occurred in " << *this << "\n";
  } else {
    Py_DECREF(result);
  }
}

#endif  // HAVE_PYTHON
