/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file windowProperties_ext.cxx
 * @author rdb
 * @date 2018-11-12
 */

#include "windowProperties_ext.h"

#ifdef HAVE_PYTHON

extern struct Dtool_PyTypedObject Dtool_WindowProperties;

/**
 * Creates a new WindowProperties initialized with the given properties.
 */
void Extension<WindowProperties>::
__init__(PyObject *self, PyObject *args, PyObject *kwds) {
  nassertv_always(_this != nullptr);

  // We need to initialize the self object before we can use it.
  DtoolInstance_INIT_PTR(self, _this);

  // Support copy constructor by extracting the one positional argument.
  Py_ssize_t nargs = PyTuple_GET_SIZE(args);
  if (nargs != 0) {
    if (nargs != 1) {
      PyErr_Format(PyExc_TypeError,
                   "WindowProperties() takes at most 1 positional argument (%d given)",
                   (int)nargs);
      return;
    }

    PyObject *arg = PyTuple_GET_ITEM(args, 0);
    const WindowProperties *copy_from;
    if (DtoolInstance_GetPointer(arg, copy_from, Dtool_WindowProperties)) {
      *_this = *copy_from;
    } else {
      Dtool_Raise_ArgTypeError(arg, 0, "WindowProperties", "WindowProperties");
      return;
    }
  }

  // Now iterate over the keyword arguments, which define the default values
  // for the different properties.
  if (kwds != nullptr) {
    __setstate__(self, kwds);
  }
}

/**
 * Returns the properties as a dictionary.
 */
PyObject *Extension<WindowProperties>::
__getstate__(PyObject *self) const {
  static const char *props[] = {"origin", "size", "title", "undecorated", "fixed_size", "fullscreen", "foreground", "minimized", "maximized", "raw_mice", "open", "cursor_hidden", "icon_filename", "cursor_filename", "z_order", "mouse_mode", "parent_window", nullptr};

  PyTypeObject *type = Py_TYPE(self);
  PyObject *state = PyDict_New();

  for (size_t i = 0; props[i] != nullptr; ++i) {
    PyObject *key = PyUnicode_FromString(props[i]);
    PyObject *descr = _PyType_Lookup(type, key);

    if (descr != nullptr && Py_TYPE(descr)->tp_descr_get != nullptr) {
      PyObject *value = Py_TYPE(descr)->tp_descr_get(descr, self, (PyObject *)type);
      nassertr(value != nullptr, nullptr);
      if (value != Py_None) {
        PyDict_SetItem(state, key, value);
      }
      Py_DECREF(value);
    }
    Py_DECREF(key);
  }

  return state;
}

/**
 *
 */
void Extension<WindowProperties>::
__setstate__(PyObject *self, PyObject *props) {
  PyTypeObject *type = Py_TYPE(self);
  PyObject *key, *value;
  Py_ssize_t pos = 0;

  Py_BEGIN_CRITICAL_SECTION(props);
  while (PyDict_Next(props, &pos, &key, &value)) {
    // Look for a writable property on the type by this name.
    PyObject *descr = _PyType_Lookup(type, key);

    if (descr != nullptr && Py_TYPE(descr)->tp_descr_set != nullptr) {
      if (Py_TYPE(descr)->tp_descr_set(descr, self, value) < 0) {
        break;
      }
    } else {
      PyObject *key_repr = PyObject_Repr(key);
      PyErr_Format(PyExc_TypeError,
                   "%.100s is an invalid keyword argument for WindowProperties()",
                   PyUnicode_AsUTF8(key_repr)
                  );
      Py_DECREF(key_repr);
      break;
    }
  }
  Py_END_CRITICAL_SECTION();
}

#endif  // HAVE_PYTHON
