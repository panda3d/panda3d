/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file bamReader_ext.cxx
 * @author rdb
 * @date 2016-04-09
 */

#include "bamReader_ext.h"
#include "config_putil.h"
#include "pythonThread.h"

#ifdef HAVE_PYTHON

#ifndef CPPPARSER
extern Dtool_PyTypedObject Dtool_BamReader;
extern Dtool_PyTypedObject Dtool_DatagramIterator;
extern Dtool_PyTypedObject Dtool_TypedWritable;
#endif  // CPPPARSER

/**
 * Factory function that calls the registered Python function.
 */
static TypedWritable *factory_callback(const FactoryParams &params){
  PyObject *func = (PyObject *)params.get_user_data();
  nassertr(func != nullptr, nullptr);

#if defined(HAVE_THREADS) && !defined(SIMPLE_THREADS)
  // Use PyGILState to protect this asynchronous call.
  PyGILState_STATE gstate;
  gstate = PyGILState_Ensure();
#endif

  // Extract the parameters we will pass to the Python function.
  DatagramIterator scan;
  BamReader *manager;
  parse_params(params, scan, manager);

  PyObject *py_scan = DTool_CreatePyInstance(&scan, Dtool_DatagramIterator, false, false);
  PyObject *py_manager = DTool_CreatePyInstance(manager, Dtool_BamReader, false, false);
  PyObject *args = PyTuple_Pack(2, py_scan, py_manager);

  // Now call the Python function.
  PyObject *result = PythonThread::call_python_func(func, args);
  Py_DECREF(args);
  Py_DECREF(py_scan);
  Py_DECREF(py_manager);

  if (result == nullptr) {
    util_cat.error()
      << "Exception occurred in Python factory function\n";

  } else if (result == Py_None) {
    util_cat.error()
      << "Python factory function returned None\n";
    Py_DECREF(result);
    result = nullptr;
  }

#if defined(HAVE_THREADS) && !defined(SIMPLE_THREADS)
  PyGILState_Release(gstate);
#endif

  // Unwrap the returned TypedWritable object.
  if (result == nullptr) {
    return nullptr;
  } else {
    void *object = nullptr;
    Dtool_Call_ExtractThisPointer(result, Dtool_TypedWritable, &object);

    TypedWritable *ptr = (TypedWritable *)object;
    ReferenceCount *ref_count = ptr->as_reference_count();
    if (ref_count != nullptr) {
      // If the Python pointer is the last reference to it, make sure that the
      // object isn't destroyed.  We do this by calling unref(), which
      // decreases the reference count without destroying the object.
      if (result->ob_refcnt <= 1) {
        ref_count->unref();

        // Tell the Python wrapper object that it shouldn't try to delete the
        // object when it is destroyed.
        ((Dtool_PyInstDef *)result)->_memory_rules = false;
      }
      Py_DECREF(result);
    }

    return (TypedWritable *)object;
  }
}

/**
 * Returns the version number of the Bam file currently being read.
 */
PyObject *Extension<BamReader>::
get_file_version() const {
  return Py_BuildValue("(ii)", _this->get_file_major_ver(),
                               _this->get_file_minor_ver());
}

/**
 * Registers a Python function as factory function for the given type.  This
 * should be a function (or class object) that takes a DatagramIterator and a
 * BamReader as arguments, and should return a TypedWritable object.
 */
void Extension<BamReader>::
register_factory(TypeHandle handle, PyObject *func) {
  nassertv(func != nullptr);

  if (!PyCallable_Check(func)) {
    Dtool_Raise_TypeError("second argument to register_factory must be callable");
    return;
  }

  Py_INCREF(func);
  BamReader::get_factory()->register_factory(handle, &factory_callback, (void *)func);
}

#endif
