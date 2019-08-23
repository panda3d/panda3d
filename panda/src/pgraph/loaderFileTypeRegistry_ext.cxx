/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file loaderFileTypeRegistry_ext.cxx
 * @author rdb
 * @date 2019-07-30
 */

#include "loaderFileTypeRegistry_ext.h"

#ifdef HAVE_PYTHON

#include "pythonLoaderFileType.h"

extern struct Dtool_PyTypedObject Dtool_LoaderFileType;

/**
 * Registers a loader file type that is implemented in Python.
 */
void Extension<LoaderFileTypeRegistry>::
register_type(PyObject *type) {
  PythonLoaderFileType *loader = new PythonLoaderFileType();

  if (loader->init(type)) {
    _this->register_type(loader);
  } else {
    delete loader;
  }
}

/**
 * Registers a loader file type from a pkg_resources.EntryPoint object, which
 * will be loaded when a file with the extension is encountered.
 */
void Extension<LoaderFileTypeRegistry>::
register_deferred_type(PyObject *entry_point) {
  // The "name" attribute holds the extension.
  PyObject *name = PyObject_GetAttrString(entry_point, "name");
  if (name == nullptr) {
    Dtool_Raise_TypeError("entry_point argument is missing name attribute");
    return;
  }

  const char *name_str;
  Py_ssize_t name_len;
#if PY_MAJOR_VERSION >= 3
  name_str = PyUnicode_AsUTF8AndSize(name, &name_len);
#else
  if (PyString_AsStringAndSize(name, (char **)&name_str, &name_len) == -1) {
    name_str = nullptr;
  }
#endif
  Py_DECREF(name);

  if (name_str == nullptr) {
    Dtool_Raise_TypeError("entry_point.name is expected to be str");
    return;
  }

  PythonLoaderFileType *loader = new PythonLoaderFileType(std::string(name_str, name_len), entry_point);
  _this->register_type(loader);
}

/**
 * If the given loader type is registered, unregisters it.
 */
void Extension<LoaderFileTypeRegistry>::
unregister_type(PyObject *type) {
  // Are we passing in a C++ file type object?
  LoaderFileType *extracted_type;
  if (DtoolInstance_GetPointer(type, extracted_type, Dtool_LoaderFileType)) {
    _this->unregister_type(extracted_type);
    return;
  }

  // If not, we may be passing in a Python file type.
  PyObject *load_func = PyObject_GetAttrString(type, "load_file");
  PyObject *save_func = PyObject_GetAttrString(type, "save_file");
  PyErr_Clear();

  if (load_func == nullptr && save_func == nullptr) {
    Dtool_Raise_TypeError("expected loader type");
    return;
  }

  // Keep looping until we've removed all instances of it.
  bool found_any;
  do {
    found_any = false;
    size_t num_types = _this->get_num_types();
    for (size_t i = 0; i < num_types; ++i) {
      LoaderFileType *type = _this->get_type(i);
      if (type->is_of_type(PythonLoaderFileType::get_class_type())) {
        PythonLoaderFileType *python_type = (PythonLoaderFileType *)type;
        if (python_type->_load_func == load_func &&
            python_type->_save_func == save_func) {
          _this->unregister_type(python_type);
          delete python_type;
          found_any = true;
          break;
        }
      }
    }
  } while (found_any);

  Py_XDECREF(load_func);
  Py_XDECREF(save_func);
}

#endif
