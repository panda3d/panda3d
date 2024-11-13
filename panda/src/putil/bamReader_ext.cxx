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
      << "Exception occurred in Python factory function:\n";
    PyErr_Print();
  }
  else if (result == Py_None) {
    util_cat.error()
      << "Python factory function returned None\n";
    Py_DECREF(result);
    result = nullptr;
  }

  void *object = nullptr;

  // Unwrap the returned TypedWritable object.
  if (result != nullptr) {
    Dtool_Call_ExtractThisPointer(result, Dtool_TypedWritable, &object);

    TypedWritable *ptr = (TypedWritable *)object;
    ReferenceCount *ref_count = ptr->as_reference_count();
    if (ref_count != nullptr) {
      // If the Python pointer is the last reference to it, make sure that the
      // object isn't destroyed.  We do this by calling unref(), which
      // decreases the reference count without destroying the object.
      if (Py_REFCNT(result) <= 1) {
        ref_count->unref();

        // Tell the Python wrapper object that it shouldn't try to delete the
        // object when it is destroyed.
        ((Dtool_PyInstDef *)result)->_memory_rules = false;
      }
      Py_DECREF(result);
    }
    else if (DtoolInstance_TYPE(result) == &Dtool_TypedWritable &&
             !Py_IS_TYPE(result, Dtool_GetPyTypeObject(&Dtool_TypedWritable))) {
      // It is a custom subclass of TypedWritable, so we have to keep it
      // alive, and decrement it in finalize(), see typedWritable_ext.cxx.
      manager->register_finalize(ptr);
    }
    else {
      // Otherwise, we just decrement the Python reference count, but making
      // sure that the C++ object is not getting deleted (yet) by this.
      bool mem_rules = false;
      std::swap(mem_rules, ((Dtool_PyInstDef *)result)->_memory_rules);
      Py_DECREF(result);
      std::swap(mem_rules, ((Dtool_PyInstDef *)result)->_memory_rules);
    }
  }

#if defined(HAVE_THREADS) && !defined(SIMPLE_THREADS)
  PyGILState_Release(gstate);
#endif

  return (TypedWritable *)object;
}

/**
 * Reads an object from the BamReader.
 */
PyObject *Extension<BamReader>::
read_object() {
  TypedWritable *ptr;
  ReferenceCount *ref_ptr;

#if defined(HAVE_THREADS) && !defined(SIMPLE_THREADS)
  PyThreadState *_save;
  Py_UNBLOCK_THREADS
#endif

  bool success = _this->read_object(ptr, ref_ptr);

#if defined(HAVE_THREADS) && !defined(SIMPLE_THREADS)
  Py_BLOCK_THREADS
#endif

  if (!success) {
    if (_this->is_eof()) {
      PyErr_SetNone(PyExc_EOFError);
      return nullptr;
    }
    return nullptr;
  }

  if (ptr == nullptr) {
    return Py_NewRef(Py_None);
  }

  if (ref_ptr != nullptr) {
    ref_ptr->ref();
  }

  // Note that, unlike the regular bindings, we take ownership of the object
  // here even if it's not inheriting from ReferenceCount.
  return DTool_CreatePyInstanceTyped((void *)ptr, Dtool_TypedWritable,
                                     true, false, ptr->get_type_index());
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
  if (!PyCallable_Check(func)) {
    Dtool_Raise_TypeError("second argument to register_factory must be callable");
    return;
  }

  void *user_data = (void *)Py_NewRef(func);
  BamReader::get_factory()->register_factory(handle, &factory_callback, user_data);
}

#endif
