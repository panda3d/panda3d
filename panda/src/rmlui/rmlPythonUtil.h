/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file rmlPythonUtil.h
 * @author tkfoss
 * @date 2026-06-18
 */

#ifndef RML_PYTHON_UTIL_H
#define RML_PYTHON_UTIL_H

#include "config_rmlui.h"

#if defined(HAVE_PYTHON) && !defined(CPPPARSER)

#include "py_panda.h"
#include <RmlUi/Core/Variant.h>
#include <limits>

/**
 * Drops a reference to a PyObject, acquiring the GIL first.  Suitable as a
 * std::shared_ptr deleter for Python callables captured by RmlUi callbacks,
 * which may be released from a thread that does not hold the GIL.
 */
static inline void
rml_py_decref_with_gil(PyObject *obj) {
  if (obj != nullptr) {
    PyGILState_STATE gs = PyGILState_Ensure();
    Py_DECREF(obj);
    PyGILState_Release(gs);
  }
}

/**
 * Converts an Rml::Variant to a new reference to a Python object.  Unsupported
 * variant types map to None.  The caller must hold the GIL.
 */
static inline PyObject *
rml_variant_to_python(const Rml::Variant &v) {
  switch (v.GetType()) {
  case Rml::Variant::BOOL:
    return PyBool_FromLong(v.Get<bool>() ? 1 : 0);
  case Rml::Variant::BYTE:
  case Rml::Variant::CHAR:
  case Rml::Variant::INT:
    return PyLong_FromLong(v.Get<int>());
  case Rml::Variant::INT64:
    return PyLong_FromLongLong(v.Get<int64_t>());
  case Rml::Variant::UINT:
    return PyLong_FromUnsignedLong(v.Get<unsigned int>());
  case Rml::Variant::UINT64:
    return PyLong_FromUnsignedLongLong(v.Get<uint64_t>());
  case Rml::Variant::FLOAT:
    return PyFloat_FromDouble(v.Get<float>());
  case Rml::Variant::DOUBLE:
    return PyFloat_FromDouble(v.Get<double>());
  case Rml::Variant::STRING:
    {
      Rml::String s = v.Get<Rml::String>();
      return PyUnicode_FromStringAndSize(s.data(), s.size());
    }
  default:
    Py_RETURN_NONE;
  }
}

/**
 * Converts a Python object into an Rml::Variant.  Supports None, bool, int,
 * float, and str; other types leave out unchanged.  A Python int that does not
 * fit in 64 bits falls back to a double (and is thus approximate).  The caller
 * must hold the GIL.
 */
static inline void
rml_python_to_variant(PyObject *obj, Rml::Variant &out) {
  if (obj == Py_None) {
    out.Clear();
  } else if (PyBool_Check(obj)) {
    out = (obj == Py_True);
  } else if (PyLong_Check(obj)) {
    // Use the full 64-bit range (Rml::Variant supports int64) and handle
    // out-of-range values explicitly so no OverflowError is left set on the
    // interpreter for the next Python call to trip over.
    int overflow = 0;
    long long ll = PyLong_AsLongLongAndOverflow(obj, &overflow);
    if (overflow != 0) {
      double d = PyLong_AsDouble(obj);
      if (d == -1.0 && PyErr_Occurred()) {
        // Exceeds double range as well; clamp rather than leaving an
        // OverflowError pending for an unrelated later Python call.
        PyErr_Clear();
        d = (overflow > 0) ? std::numeric_limits<double>::infinity()
                           : -std::numeric_limits<double>::infinity();
      }
      out = d;
    } else {
      out = (int64_t)ll;
    }
  } else if (PyFloat_Check(obj)) {
    out = (double)PyFloat_AsDouble(obj);
  } else if (PyUnicode_Check(obj)) {
    Py_ssize_t len;
    const char *s = PyUnicode_AsUTF8AndSize(obj, &len);
    if (s != nullptr) {
      out = Rml::String(s, (size_t)len);
    }
  }
}

#endif  // HAVE_PYTHON && !CPPPARSER

#endif
