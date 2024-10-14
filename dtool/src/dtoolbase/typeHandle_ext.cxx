/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file typeHandle_ext.cxx
 * @author rdb
 * @date 2014-09-17
 */

#include "typeHandle_ext.h"

#ifdef HAVE_PYTHON

/**
 * Constructs a TypeHandle from a Python class object.  Useful for automatic
 * coercion, to allow a class object to be passed wherever a TypeHandle is
 * expected.
 */
TypeHandle Extension<TypeHandle>::
make(PyTypeObject *tp) {
  Dtool_PyTypedObject *super_base = Dtool_GetSuperBase();
  if (!PyType_IsSubtype(tp, (PyTypeObject *)super_base)) {
    PyErr_SetString(PyExc_TypeError, "a Panda type is required");
    return TypeHandle::none();
  }

  Dtool_PyTypedObject *dtool_tp = (Dtool_PyTypedObject *) tp;
  return dtool_tp->_type;
}

/**
 * Implements pickle support.
 */
PyObject *Extension<TypeHandle>::
__reduce__() const {
  extern struct Dtool_PyTypedObject Dtool_TypeHandle;

  if (!*_this) {
    PyObject *func = PyObject_GetAttrString((PyObject *)&Dtool_TypeHandle, "none");
    return Py_BuildValue("N()", func);
  }

  // If we have a Python binding registered for it, that's the preferred method,
  // since it ensures that the appropriate module gets loaded by pickle.
  PyTypeObject *py_type = _this->get_python_type();
  if (py_type != nullptr && py_type->tp_dict != nullptr) {
    // Look for a get_class_type method, if it returns this handle.
    PyObject *func;
    int result = PyDict_GetItemStringRef(py_type->tp_dict, "get_class_type", &func);
    if (result > 0) {
      if (PyCallable_Check(func)) {
        PyObject *result = PyObject_CallNoArgs(func);
        TypeHandle *result_handle = nullptr;
        if (result == nullptr) {
          // Never mind.
          PyErr_Clear();
        }
        else if (DtoolInstance_GetPointer(result, result_handle, Dtool_TypeHandle) &&
                 *result_handle == *_this) {
          // It returned the correct result, so we can use this.
          return Py_BuildValue("N()", func);
        }
      }
      Py_DECREF(func);
    }
    else if (result < 0) {
      PyErr_Clear();
    }
  }

  // Fall back to TypeHandle::make(), if would produce the correct result.
  if (py_type != nullptr && *_this == ((Dtool_PyTypedObject *)py_type)->_type) {
    PyObject *func = PyObject_GetAttrString((PyObject *)&Dtool_TypeHandle, "make");
    return Py_BuildValue("N(O)", func, py_type);
  }

  // Fall back to the __setstate__ mechanism.
  std::string name = _this->get_name();
  Py_ssize_t num_parents = _this->get_num_parent_classes();
  PyObject *parents = PyTuple_New(num_parents);
  for (Py_ssize_t i = 0; i < num_parents; ++i) {
    PyObject *parent = DTool_CreatePyInstance(new TypeHandle(_this->get_parent_class(i)), Dtool_TypeHandle, true, false);
    PyTuple_SET_ITEM(parents, i, parent);
  }
  return Py_BuildValue("O()(s#N)", (PyObject *)&Dtool_TypeHandle, name.c_str(), name.size(), parents);
}

/**
 * Implements pickle support.
 */
void Extension<TypeHandle>::
__setstate__(PyObject *state) {
  Py_ssize_t len;
  const char *name_str = PyUnicode_AsUTF8AndSize(PyTuple_GET_ITEM(state, 0), &len);
  PyObject *parents = PyTuple_GET_ITEM(state, 1);

  TypeRegistry *type_registry = TypeRegistry::ptr();
  *_this = type_registry->register_dynamic_type(std::string(name_str, len));

  Py_ssize_t num_parents = PyTuple_GET_SIZE(parents);
  for (Py_ssize_t i = 0; i < num_parents; ++i) {
    TypeHandle *parent = (TypeHandle *)DtoolInstance_VOID_PTR(PyTuple_GET_ITEM(parents, i));
    type_registry->record_derivation(*_this, *parent);
  }
}

#endif
