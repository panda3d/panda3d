/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file pythonCallbackObject.cxx
 * @author drose
 * @date 2009-03-13
 */

#include "pythonCallbackObject.h"

#ifdef HAVE_PYTHON

#include "py_panda.h"
#include "pythonThread.h"
#include "callbackData.h"
#include "config_putil.h"

TypeHandle PythonCallbackObject::_type_handle;

Configure(config_pythonCallbackObject);
ConfigureFn(config_pythonCallbackObject) {
  PythonCallbackObject::init_type();
}

#ifndef CPPPARSER
extern struct Dtool_PyTypedObject Dtool_TypedObject;
extern struct Dtool_PyTypedObject Dtool_PythonCallbackObject;
#endif

/**
 *
 */
PythonCallbackObject::
PythonCallbackObject(PyObject *function) {
  _function = Py_NewRef(Py_None);

  set_function(function);

#if !defined(SIMPLE_THREADS) && defined(WITH_THREAD)
  // Ensure that the Python threading system is initialized and ready to go.
  // WITH_THREAD symbol defined within Python.h
  Py_Initialize();

#if PY_VERSION_HEX < 0x03090000
  // PyEval_InitThreads is now a deprecated no-op in Python 3.9+
  PyEval_InitThreads();
#endif // PY_VERSION_HEX
#endif // WITH_THREAD
}

/**
 *
 */
PythonCallbackObject::
~PythonCallbackObject() {
  Py_DECREF(_function);
}

/**
 * Replaces the function that is called for the callback.  runs.  The
 * parameter should be a Python callable object.
 */
void PythonCallbackObject::
set_function(PyObject *function) {
  Py_DECREF(_function);
  _function = Py_NewRef(function);
  if (_function != Py_None && !PyCallable_Check(_function)) {
    nassert_raise("Invalid function passed to PythonCallbackObject");
  }
}

/**
 * Returns the function that is called for the callback.
 */
PyObject *PythonCallbackObject::
get_function() {
  return Py_NewRef(_function);
}

/**
 * Implements pickle support.
 */
PyObject *PythonCallbackObject::
__reduce__() const {
  return Py_BuildValue("O(O)", (PyObject *)Dtool_GetPyTypeObject(&Dtool_PythonCallbackObject), _function);
}

/**
 * This method called when the callback is triggered; it *replaces* the
 * original function.  To continue performing the original function, you must
 * call cbdata->upcall() during the callback.
 */
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

/**
 * The Python calls that implement do_callback().  This function is separate
 * so we can acquire the Python interpretor lock while it runs.
 */
void PythonCallbackObject::
do_python_callback(CallbackData *cbdata) {
  nassertv(cbdata != nullptr);

  // Wrap the cbdata up in a Python object, then put it in a tuple, for the
  // argument list.
  PyObject *pycbdata =
    DTool_CreatePyInstanceTyped(cbdata, Dtool_TypedObject,
                                false, false, cbdata->get_type_index());
  PyObject *args = Py_BuildValue("(O)", pycbdata);
  Py_DECREF(pycbdata);

  PyObject *result = PythonThread::call_python_func(_function, args);
  Py_DECREF(args);

  if (result == nullptr) {
    if (PyErr_Occurred() != PyExc_SystemExit) {
      util_cat.error()
        << "Exception occurred in " << *this << "\n";
    }
  } else {
    Py_DECREF(result);
  }
}

#endif  // HAVE_PYTHON
