/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file typedWritable_ext.cxx
 * @author rdb
 * @date 2013-12-10
 */

#include "typedWritable_ext.h"

#ifdef HAVE_PYTHON

#include "bamWriter.h"
#include "config_putil.h"

#ifndef CPPPARSER
extern Dtool_PyTypedObject Dtool_BamReader;
extern Dtool_PyTypedObject Dtool_BamWriter;
extern Dtool_PyTypedObject Dtool_Datagram;
extern Dtool_PyTypedObject Dtool_DatagramIterator;
extern Dtool_PyTypedObject Dtool_TypedObject;
extern Dtool_PyTypedObject Dtool_TypedWritable;
extern Dtool_PyTypedObject Dtool_TypeHandle;
#endif  // CPPPARSER

/**
 * Class that upcalls to the parent class when write_datagram is called.
 */
class TypedWritableProxy : public TypedWritable, public DtoolProxy {
public:
  ~TypedWritableProxy() {
  }

  virtual void write_datagram(BamWriter *manager, Datagram &dg) override {
    // The derived method may call back to the TypedWritable implementation,
    // which would end up back here.  Detect and prevent this.
    thread_local TypedWritableProxy *recursion_protect = nullptr;
    if (recursion_protect == this) {
      TypedWritable::write_datagram(manager, dg);
      return;
    }

    // We don't know where this might be invoked, so we have to be on the safe
    // side and ensure that the GIL is being held.
#if defined(HAVE_THREADS) && !defined(SIMPLE_THREADS)
    PyGILState_STATE gstate = PyGILState_Ensure();
#endif

    TypedWritableProxy *prev_recursion_protect = this;
    std::swap(recursion_protect, prev_recursion_protect);

    PyObject *py_manager = DTool_CreatePyInstance(manager, Dtool_BamWriter, false, false);
    PyObject *py_dg = DTool_CreatePyInstance(&dg, Dtool_Datagram, false, false);

    PyObject *result = PyObject_CallMethod(_self, "write_datagram", "NN", py_manager, py_dg);
    if (result != nullptr) {
      Py_DECREF(result);
    } else {
      util_cat.error()
        << "Exception occurred in Python write_datagram function:\n";
      PyErr_Print();
    }

    std::swap(recursion_protect, prev_recursion_protect);

#if defined(HAVE_THREADS) && !defined(SIMPLE_THREADS)
    PyGILState_Release(gstate);
#endif
  }

  virtual void fillin(DatagramIterator &scan, BamReader *manager) override {
    // The derived method may call back to the TypedWritable implementation,
    // which would end up back here.  Detect and prevent this.
    thread_local TypedWritableProxy *recursion_protect = nullptr;
    if (recursion_protect == this) {
      TypedWritable::fillin(scan, manager);
      return;
    }

    // We don't know where this might be invoked, so we have to be on the safe
    // side and ensure that the GIL is being held.
#if defined(HAVE_THREADS) && !defined(SIMPLE_THREADS)
    PyGILState_STATE gstate = PyGILState_Ensure();
#endif

    TypedWritableProxy *prev_recursion_protect = this;
    std::swap(recursion_protect, prev_recursion_protect);

    PyObject *py_scan = DTool_CreatePyInstance(&scan, Dtool_DatagramIterator, false, false);
    PyObject *py_manager = DTool_CreatePyInstance(manager, Dtool_BamReader, false, false);

    PyObject *result = PyObject_CallMethod(_self, "fillin", "NN", py_scan, py_manager);
    if (result != nullptr) {
      Py_DECREF(result);
    } else {
      util_cat.error()
        << "Exception occurred in Python fillin function:\n";
      PyErr_Print();
    }

    std::swap(recursion_protect, prev_recursion_protect);

#if defined(HAVE_THREADS) && !defined(SIMPLE_THREADS)
    PyGILState_Release(gstate);
#endif
  }

  virtual void finalize(BamReader *manager) override {
    // If nobody stored the object after calling read_object(), this will cause
    // this object to be deleted.
    Py_DECREF(_self);
  }

  virtual TypeHandle get_type() const override {
    return _type;
  }

  virtual TypeHandle force_init_type() override {
    return _type;
  }
};

/**
 * Returns a wrapper object for a TypedWritable subclass.
 */
static PyObject *
wrap_typed_writable(void *from_this, PyTypeObject *from_type) {
  nassertr(from_this != nullptr, nullptr);
  nassertr(from_type != nullptr, nullptr);

  TypedWritableProxy *to_this;
  if (from_type == Dtool_GetPyTypeObject(&Dtool_TypedWritable)) {
    to_this = (TypedWritableProxy *)(TypedWritable *)from_this;
  }
  else if (from_type == Dtool_GetPyTypeObject(&Dtool_TypedObject)) {
    to_this = (TypedWritableProxy *)(TypedObject *)from_this;
  }
  else {
    return nullptr;
  }

  nassertr(to_this->_self != nullptr, nullptr);
  return Py_NewRef(to_this->_self);
}

/**
 * Registers a Python type recursively, towards the TypedWritable base.
 * Returns a TypeHandle if it inherited from TypedWritable, 0 otherwise.
 */
static TypeHandle
register_python_type(TypeRegistry *registry, PyTypeObject *cls) {
  TypeHandle handle = TypeHandle::none();

  if (cls->tp_bases != nullptr) {
    Py_ssize_t count = PyTuple_GET_SIZE(cls->tp_bases);
    for (Py_ssize_t i = 0; i < count; ++i) {
      PyObject *base = PyTuple_GET_ITEM(cls->tp_bases, count);
      TypeHandle base_handle = register_python_type(registry, (PyTypeObject *)base);
      if (base_handle != TypeHandle::none()) {
        if (handle == TypeHandle::none()) {
          handle = registry->register_dynamic_type(cls->tp_name);
        }
        registry->record_derivation(handle, base_handle);
        return handle;
      }
    }
  }

  return handle;
}

/**
 * This is called when a TypedWritable is instantiated directly.
 */
PyObject *Extension<TypedWritable>::
__new__(PyTypeObject *cls) {
  if (cls == (PyTypeObject *)&Dtool_TypedWritable) {
    return Dtool_Raise_TypeError("cannot init abstract class");
  }

  PyObject *self = cls->tp_alloc(cls, 0);
  ((Dtool_PyInstDef *)self)->_signature = PY_PANDA_SIGNATURE;
  ((Dtool_PyInstDef *)self)->_My_Type = &Dtool_TypedWritable;

  // We expect the user to override this method.
  PyObject *class_type = PyObject_CallMethod((PyObject *)cls, "get_class_type", nullptr);
  if (class_type == nullptr) {
    return nullptr;
  }

  // Check that it returned a TypeHandle, and that it is actually different
  // from the one on the base class (which might mean that the user didn't
  // actually define a custom get_class_type() method).
  TypeHandle *handle = nullptr;
  if (!DtoolInstance_GetPointer(class_type, handle, Dtool_TypeHandle) ||
      *handle == TypedWritable::get_class_type() ||
      *handle == TypedObject::get_class_type() ||
      !handle->is_derived_from(TypedWritable::get_class_type())) {
    Dtool_Raise_TypeError("get_class_type() must be overridden to return a unique TypeHandle that indicates derivation from TypedWritable");
    return nullptr;
  }

  // Make sure that the bindings know how to obtain a wrapper for this type.
  TypeRegistry *registry = TypeRegistry::ptr();
  PyTypeObject *cls_ref = (PyTypeObject *)Py_NewRef((PyObject *)cls);
  registry->record_python_type(*handle, cls_ref, &wrap_typed_writable);

  // Note that we don't increment the reference count here, because that would
  // create a memory leak.  The TypedWritableProxy gets deleted when the Python
  // object reaches a reference count of 0.
  TypedWritableProxy *proxy = new TypedWritableProxy;
  proxy->_self = self;
  proxy->_type = *handle;

  DTool_PyInit_Finalize(self, (void *)proxy, &Dtool_TypedWritable, true, false);
  return self;
}

/**
 * This special Python method is implement to provide support for the pickle
 * module.
 *
 * This hooks into the native pickle and cPickle modules, but it cannot
 * properly handle self-referential BAM objects.
 */
PyObject *Extension<TypedWritable>::
__reduce__(PyObject *self) const {
  return __reduce_persist__(self, nullptr);
}

/**
 * This special Python method is implement to provide support for the pickle
 * module.
 *
 * This is similar to __reduce__, but it provides additional support for the
 * missing persistent-state object needed to properly support self-referential
 * BAM objects written to the pickle stream.  This hooks into the pickle and
 * cPickle modules implemented in direct/src/stdpy.
 */
PyObject *Extension<TypedWritable>::
__reduce_persist__(PyObject *self, PyObject *pickler) const {
  // We should return at least a 2-tuple, (Class, (args)): the necessary class
  // object whose constructor we should call (e.g.  this), and the arguments
  // necessary to reconstruct this object.

  // Check that we have a decode_from_bam_stream python method.  If not, we
  // can't use this interface.
  PyObject *method = PyObject_GetAttrString(self, "decode_from_bam_stream");
  if (method == nullptr) {
    std::ostringstream stream;
    stream << "Cannot pickle objects of type " << _this->get_type() << "\n";
    std::string message = stream.str();
    PyErr_SetString(PyExc_TypeError, message.c_str());
    return nullptr;
  }
  Py_DECREF(method);

  BamWriter *writer = nullptr;
  if (pickler != nullptr) {
    PyObject *py_writer = PyObject_GetAttrString(pickler, "bamWriter");
    if (py_writer == nullptr) {
      // It's OK if there's no bamWriter.
      PyErr_Clear();
    } else {
      DtoolInstance_GetPointer(py_writer, writer, Dtool_BamWriter);
      Py_DECREF(py_writer);
    }
  }

  // First, streamify the object, if possible.
  vector_uchar bam_stream;
  if (!_this->encode_to_bam_stream(bam_stream, writer)) {
    std::ostringstream stream;
    stream << "Could not bamify object of type " << _this->get_type() << "\n";
    std::string message = stream.str();
    PyErr_SetString(PyExc_TypeError, message.c_str());
    return nullptr;
  }

  // Start by getting this class object.
  PyObject *this_class = PyObject_Type(self);
  if (this_class == nullptr) {
    return nullptr;
  }

  PyObject *func;
  if (writer != nullptr) {
    // The modified pickle support: call the "persistent" version of this
    // function, which receives the unpickler itself as an additional
    // parameter.
    func = find_global_decode(this_class, "py_decode_TypedWritable_from_bam_stream_persist");
    if (func == nullptr) {
      PyErr_SetString(PyExc_TypeError, "Couldn't find py_decode_TypedWritable_from_bam_stream_persist()");
      Py_DECREF(this_class);
      return nullptr;
    }

  } else {
    // The traditional pickle support: call the non-persistent version of this
    // function.
    func = find_global_decode(this_class, "py_decode_TypedWritable_from_bam_stream");
    if (func == nullptr) {
      PyErr_SetString(PyExc_TypeError, "Couldn't find py_decode_TypedWritable_from_bam_stream()");
      Py_DECREF(this_class);
      return nullptr;
    }
  }

  // PyTuple_SET_ITEM conveniently borrows the reference it is passed.
  PyObject *args = PyTuple_New(2);
  PyTuple_SET_ITEM(args, 0, this_class);
  PyTuple_SET_ITEM(args, 1, Dtool_WrapValue(bam_stream));

  PyObject *tuple = PyTuple_New(2);
  PyTuple_SET_ITEM(tuple, 0, func);
  PyTuple_SET_ITEM(tuple, 1, args);
  return tuple;
}

/**
 * This is a support function for __reduce__().  It searches for the global
 * function py_decode_TypedWritable_from_bam_stream() in this class's module,
 * or in the module for any base class.  (It's really looking for the libpanda
 * module, but we can't be sure what name that module was loaded under, so we
 * search upwards this way.)
 *
 * Returns: new reference on success, or NULL on failure.
 */
PyObject *Extension<TypedWritable>::
find_global_decode(PyObject *this_class, const char *func_name) {
  // Get the module in which BamWriter is defined.
  PyObject *module_name = PyObject_GetAttrString((PyObject *)&Dtool_BamWriter, "__module__");
  if (module_name != nullptr) {
    PyObject *module = PyImport_GetModule(module_name);
    Py_DECREF(module_name);
    if (module != nullptr) {
      PyObject *func = PyObject_GetAttrString(module, (char *)func_name);
      Py_DECREF(module);
      if (func != nullptr) {
        return func;
      }
    }
  }

  PyObject *bases = PyObject_GetAttrString(this_class, "__bases__");
  if (bases != nullptr) {
    {
      PyObject *tuple = PySequence_Tuple(bases);
      Py_DECREF(bases);
      if (tuple == nullptr) {
        PyErr_Clear();
        return nullptr;
      }
      bases = tuple;
    }

    Py_ssize_t size = PyTuple_GET_SIZE(bases);
    for (Py_ssize_t i = 0; i < size; ++i) {
      PyObject *base = PyTuple_GET_ITEM(bases, i);
      if (base != nullptr) {
        PyObject *func = find_global_decode(base, func_name);
        if (func != nullptr) {
          Py_DECREF(bases);
          return func;
        }
      }
    }
    Py_DECREF(bases);
  }

  return nullptr;
}

/**
 * This wrapper is defined as a global function to suit pickle's needs.
 *
 * This hooks into the native pickle and cPickle modules, but it cannot
 * properly handle self-referential BAM objects.
 */
PyObject *
py_decode_TypedWritable_from_bam_stream(PyObject *this_class, const vector_uchar &data) {
  return py_decode_TypedWritable_from_bam_stream_persist(nullptr, this_class, data);
}

/**
 * This wrapper is defined as a global function to suit pickle's needs.
 *
 * This is similar to py_decode_TypedWritable_from_bam_stream, but it provides
 * additional support for the missing persistent-state object needed to
 * properly support self-referential BAM objects written to the pickle stream.
 * This hooks into the pickle and cPickle modules implemented in
 * direct/src/stdpy.
 */
PyObject *
py_decode_TypedWritable_from_bam_stream_persist(PyObject *pickler, PyObject *this_class, const vector_uchar &data) {

  PyObject *py_reader = nullptr;
  if (pickler != nullptr) {
    py_reader = PyObject_GetAttrString(pickler, "bamReader");
    if (py_reader == nullptr) {
      // It's OK if there's no bamReader.
      PyErr_Clear();
    }
  }

  // We need the function PandaNode::decode_from_bam_stream or
  // TypedWritableReferenceCount::decode_from_bam_stream, which invokes the
  // BamReader to reconstruct this object.  Since we use the specific object's
  // class as the pointer, we get the particular instance of
  // decode_from_bam_stream appropriate to this class.

  PyObject *func = PyObject_GetAttrString(this_class, "decode_from_bam_stream");
  if (func == nullptr) {
    Py_XDECREF(py_reader);
    return nullptr;
  }

  PyObject *bytes = Dtool_WrapValue(data);
  if (bytes == nullptr) {
    Py_DECREF(func);
    Py_XDECREF(py_reader);
    return nullptr;
  }

  PyObject *result;
  if (py_reader != nullptr) {
    result = PyObject_CallFunctionObjArgs(func, bytes, py_reader, nullptr);
    Py_DECREF(py_reader);
  } else {
    result = PyObject_CallOneArg(func, bytes);
  }
  Py_DECREF(bytes);
  Py_DECREF(func);

  if (result == nullptr) {
    return nullptr;
  }

  if (result == Py_None) {
    Py_DECREF(result);
    PyErr_SetString(PyExc_ValueError, "Could not unpack bam stream");
    return nullptr;
  }

  return result;
}

#endif
