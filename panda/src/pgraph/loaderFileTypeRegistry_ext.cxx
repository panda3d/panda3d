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

#endif
