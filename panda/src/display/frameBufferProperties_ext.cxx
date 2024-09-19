/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file frameBufferProperties_ext.cxx
 * @author rdb
 * @date 2021-12-13
 */

#include "frameBufferProperties_ext.h"

#ifdef HAVE_PYTHON

/**
 * Returns the properties as a dictionary.
 */
PyObject *Extension<FrameBufferProperties>::
__getstate__() const {
  static const char *props[FrameBufferProperties::FBP_COUNT] = {"depth_bits", "color_bits", "red_bits", "green_bits", "blue_bits", "alpha_bits", "stencil_bits", "accum_bits", "aux_rgba", "aux_hrgba", "aux_float", "multisamples", "coverage_samples", "back_buffers"};
  static const char *flags[] = {"indexed_color", "rgb_color", "stereo", "force_hardware", "force_software", "srgb_color", "float_color", "float_depth", nullptr};

  PyObject *state = PyDict_New();

  for (size_t i = 0; i < FrameBufferProperties::FBP_COUNT; ++i) {
    if (_this->_specified & (1 << i)) {
      PyObject *value = PyLong_FromLong(_this->_property[i]);
      PyDict_SetItemString(state, props[i], value);
      Py_DECREF(value);
    }
  }

  for (size_t i = 0; flags[i] != nullptr; ++i) {
    if (_this->_flags_specified & (1 << i)) {
      PyObject *value = (_this->_flags & (1 << i)) ? Py_True : Py_False;
      PyDict_SetItemString(state, flags[i], value);
    }
  }

  return state;
}

/**
 *
 */
void Extension<FrameBufferProperties>::
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
                   "%.100s is an invalid framebuffer property",
                   PyUnicode_AsUTF8(key_repr)
                  );
      Py_DECREF(key_repr);
      break;
    }
  }
  Py_END_CRITICAL_SECTION();
}

#endif  // HAVE_PYTHON
