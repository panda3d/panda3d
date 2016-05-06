/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file py_panda.I
 * @author rdb
 * @date 2016-06-06
 */

/**
 * Template function that can be used to extract any TypedObject pointer from
 * a wrapped Python object.
 */
template<class T> INLINE bool
DTOOL_Call_ExtractThisPointer(PyObject *self, T *&into) {
  if (DtoolCanThisBeAPandaInstance(self)) {
    Dtool_PyTypedObject *target_class = Dtool_RuntimeTypeDtoolType(get_type_handle(T).get_index());
    if (target_class != NULL) {
      into = (T*) ((Dtool_PyInstDef *)self)->_My_Type->_Dtool_UpcastInterface(self, target_class);
      return (into != NULL);
    }
  }
  into = NULL;
  return false;
}

/**
 * These functions wrap a pointer for a class that defines get_type_handle().
 */
template<class T> INLINE PyObject *
DTool_CreatePyInstance(const T *obj, bool memory_rules) {
  Dtool_PyTypedObject *known_class = Dtool_RuntimeTypeDtoolType(get_type_handle(T).get_index());
  nassertr(known_class != NULL, NULL);
  return DTool_CreatePyInstance((void*) obj, *known_class, memory_rules, true);
}

template<class T> INLINE PyObject *
DTool_CreatePyInstance(T *obj, bool memory_rules) {
  Dtool_PyTypedObject *known_class = Dtool_RuntimeTypeDtoolType(get_type_handle(T).get_index());
  nassertr(known_class != NULL, NULL);
  return DTool_CreatePyInstance((void*) obj, *known_class, memory_rules, false);
}

template<class T> INLINE PyObject *
DTool_CreatePyInstanceTyped(const T *obj, bool memory_rules) {
  Dtool_PyTypedObject *known_class = Dtool_RuntimeTypeDtoolType(get_type_handle(T).get_index());
  nassertr(known_class != NULL, NULL);
  return DTool_CreatePyInstanceTyped((void*) obj, *known_class, memory_rules, true, obj->get_type().get_index());
}

template<class T> INLINE PyObject *
DTool_CreatePyInstanceTyped(T *obj, bool memory_rules) {
  Dtool_PyTypedObject *known_class = Dtool_RuntimeTypeDtoolType(get_type_handle(T).get_index());
  nassertr(known_class != NULL, NULL);
  return DTool_CreatePyInstanceTyped((void*) obj, *known_class, memory_rules, false, obj->get_type().get_index());
}

/**
 * The following functions wrap an arbitrary C++ value into a PyObject.
 */
ALWAYS_INLINE PyObject *Dtool_WrapValue(int value) {
#if PY_MAJOR_VERSION >= 3
  return PyLong_FromLong((long)value);
#else
  return PyInt_FromLong((long)value);
#endif
}

ALWAYS_INLINE PyObject *Dtool_WrapValue(unsigned int value) {
#if PY_MAJOR_VERSION >= 3 && SIZEOF_INT < SIZEOF_LONG
  return PyLong_FromLong((long)value);
#elif PY_MAJOR_VERSION >= 3
  return PyLong_FromUnsignedLong((unsigned long)value);
#elif SIZEOF_INT < SIZEOF_LONG
  return PyInt_FromLong((long)value);
#else
  return (value > LONG_MAX)
    ? PyLong_FromUnsignedLong((unsigned long)value)
    : PyInt_FromLong((long)value);
#endif
}

ALWAYS_INLINE PyObject *Dtool_WrapValue(long value) {
#if PY_MAJOR_VERSION >= 3
  return PyLong_FromLong(value);
#else
  return PyInt_FromLong(value);
#endif
}

ALWAYS_INLINE PyObject *Dtool_WrapValue(unsigned long value) {
#if PY_MAJOR_VERSION >= 3
  return PyLong_FromUnsignedLong(value);
#else
  return (value > LONG_MAX)
    ? PyLong_FromUnsignedLong(value)
    : PyInt_FromLong((long)value);
#endif
}

ALWAYS_INLINE PyObject *Dtool_WrapValue(long long value) {
  return PyLong_FromLongLong(value);
}

ALWAYS_INLINE PyObject *Dtool_WrapValue(unsigned long long value) {
  return PyLong_FromUnsignedLongLong(value);
}

ALWAYS_INLINE PyObject *Dtool_WrapValue(bool value) {
  PyObject *result = (value ? Py_True : Py_False);
  Py_INCREF(result);
  return result;
}

ALWAYS_INLINE PyObject *Dtool_WrapValue(double value) {
  return PyFloat_FromDouble(value);
}

ALWAYS_INLINE PyObject *Dtool_WrapValue(const char *value) {
  if (value == (const char *)NULL) {
    Py_INCREF(Py_None);
    return Py_None;
  } else {
#if PY_MAJOR_VERSION >= 3
    return PyUnicode_FromString(value);
#else
    return PyString_FromString(value);
#endif
  }
}

ALWAYS_INLINE PyObject *Dtool_WrapValue(const wchar_t *value) {
  if (value == (const wchar_t *)NULL) {
    Py_INCREF(Py_None);
    return Py_None;
  } else {
    return PyUnicode_FromWideChar(value, (Py_ssize_t)wcslen(value));
  }
}

ALWAYS_INLINE PyObject *Dtool_WrapValue(const std::string &value) {
#if PY_MAJOR_VERSION >= 3
  return PyUnicode_FromStringAndSize(value.data(), (Py_ssize_t)value.length());
#else
  return PyString_FromStringAndSize(value.data(), (Py_ssize_t)value.length());
#endif
}

ALWAYS_INLINE PyObject *Dtool_WrapValue(const std::wstring &value) {
  return PyUnicode_FromWideChar(value.data(), (Py_ssize_t)value.length());
}

ALWAYS_INLINE PyObject *Dtool_WrapValue(const std::string *value) {
  if (value == (const std::string *)NULL) {
    Py_INCREF(Py_None);
    return Py_None;
  } else {
#if PY_MAJOR_VERSION >= 3
    return PyUnicode_FromStringAndSize(value->data(), (Py_ssize_t)value->length());
#else
    return PyString_FromStringAndSize(value->data(), (Py_ssize_t)value->length());
#endif
  }
}

ALWAYS_INLINE PyObject *Dtool_WrapValue(const std::wstring *value) {
  if (value == (const std::wstring *)NULL) {
    Py_INCREF(Py_None);
    return Py_None;
  } else {
    return PyUnicode_FromWideChar(value->data(), (Py_ssize_t)value->length());
  }
}

ALWAYS_INLINE PyObject *Dtool_WrapValue(char value) {
#if PY_MAJOR_VERSION >= 3
  return PyUnicode_FromStringAndSize(&value, 1);
#else
  return PyString_FromStringAndSize(&value, 1);
#endif
}

ALWAYS_INLINE PyObject *Dtool_WrapValue(wchar_t value) {
  return PyUnicode_FromWideChar(&value, 1);
}

ALWAYS_INLINE PyObject *Dtool_WrapValue(PyObject *value) {
  return value;
}

#if PY_MAJOR_VERSION >= 0x02060000
ALWAYS_INLINE PyObject *Dtool_WrapValue(Py_buffer *value) {
  if (value == (Py_buffer *)NULL) {
    return value;
  } else {
    return PyMemoryView_FromBuffer(value);
  }
}
#endif
