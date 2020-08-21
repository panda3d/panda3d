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

#ifndef CPPPARSER
extern Dtool_PyTypedObject Dtool_BamWriter;
#endif  // CPPPARSER

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
    // borrowed reference
    PyObject *sys_modules = PyImport_GetModuleDict();
    if (sys_modules != nullptr) {
      // borrowed reference
      PyObject *module = PyDict_GetItem(sys_modules, module_name);
      if (module != nullptr) {
        PyObject *func = PyObject_GetAttrString(module, (char *)func_name);
        if (func != nullptr) {
          Py_DECREF(module_name);
          return func;
        }
      }
    }
    Py_DECREF(module_name);
  }

  PyObject *bases = PyObject_GetAttrString(this_class, "__bases__");
  if (bases != nullptr) {
    if (PySequence_Check(bases)) {
      Py_ssize_t size = PySequence_Size(bases);
      for (Py_ssize_t i = 0; i < size; ++i) {
        PyObject *base = PySequence_GetItem(bases, i);
        if (base != nullptr) {
          PyObject *func = find_global_decode(base, func_name);
          Py_DECREF(base);
          if (func != nullptr) {
            Py_DECREF(bases);
            return func;
          }
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
    result = PyObject_CallFunctionObjArgs(func, bytes, nullptr);
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
