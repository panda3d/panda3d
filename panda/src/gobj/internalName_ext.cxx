/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file internalName_ext.cxx
 * @author rdb
 * @date 2014-09-28
 */

#include "internalName_ext.h"

using std::string;

#ifdef HAVE_PYTHON

/**
 * This extension method serves to allow coercion of Python interned strings
 * to InternalName objects more efficiently by storing a mapping between
 * Python and Panda interned strings.
 */
#if PY_MAJOR_VERSION >= 3
PT(InternalName) Extension<InternalName>::
make(PyObject *str) {
  if (!PyUnicode_Check(str)) {
    Dtool_Raise_ArgTypeError(str, 0, "InternalName.make", "str");
    return nullptr;
  }

  if (!PyUnicode_CHECK_INTERNED(str)) {
    // Not an interned string; don't bother.
    Py_ssize_t len = 0;
    const char *c_str = PyUnicode_AsUTF8AndSize((PyObject *)str, &len);
    if (c_str == nullptr) {
      return nullptr;
    }

    string name(c_str, len);
    return InternalName::make(name);
  }

  InternalName::PyInternTable::const_iterator it;
  it = InternalName::_py_intern_table.find((PyObject*)str);

  if (it != InternalName::_py_intern_table.end()) {
    return (*it).second;

  } else {
    Py_ssize_t len = 0;
    const char *c_str = PyUnicode_AsUTF8AndSize((PyObject *)str, &len);
    string name(c_str, len);

#else
PT(InternalName) Extension<InternalName>::
make(PyObject *str) {
  if (!PyString_Check(str)) {
    Dtool_Raise_ArgTypeError(str, 0, "InternalName.make", "str");
    return nullptr;
  }

  if (!PyString_CHECK_INTERNED(str)) {
    // Not an interned string; don't bother.
    string name(PyString_AS_STRING(str), PyString_GET_SIZE(str));
    return InternalName::make(name);
  }

  InternalName::PyInternTable::const_iterator it;
  it = InternalName::_py_intern_table.find((PyObject*)str);

  if (it != InternalName::_py_intern_table.end()) {
    return (*it).second;

  } else {
    string name(PyString_AS_STRING(str), PyString_GET_SIZE(str));

#endif  // PY_MAJOR_VERSION

    PT(InternalName) iname = InternalName::make(name);

    // We basically leak references to both the PyObject and the InternalName.
    // We may want to change that in the future if it becomes a problem.
    Py_INCREF(str);
    iname->ref();

    InternalName::_py_intern_table.insert(std::make_pair((PyObject *)str, iname.p()));
    return iname;
  }

}

#endif  // HAVE_PYTHON
