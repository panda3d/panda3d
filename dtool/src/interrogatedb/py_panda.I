/**
 * @file py_panda.I
 * @author rdb
 * @date 2016-06-06
 */

#ifdef _MSC_VER
#define _IS_FINAL(T) (__is_sealed(T))
#elif defined(__GNUC__)
#define _IS_FINAL(T) (__is_final(T))
#else
#define _IS_FINAL(T) (0)
#endif

/**
 * Template function that can be used to extract any TypedObject pointer from
 * a wrapped Python object.
 */
template<class T> INLINE bool
DtoolInstance_GetPointer(PyObject *self, T *&into) {
  if (DtoolInstance_Check(self)) {
    Dtool_PyTypedObject *target_class = (Dtool_PyTypedObject *)get_type_handle(T).get_python_type();
    if (target_class != nullptr) {
      if (_IS_FINAL(T)) {
        if (DtoolInstance_TYPE(self) == target_class) {
          into = (T *)DtoolInstance_VOID_PTR(self);
        } else {
          return false;
        }
      } else {
        into = (T *)DtoolInstance_UPCAST(self, *target_class);
      }
      return (into != nullptr);
    }
  }
  into = nullptr;
  return false;
}

/**
 * Template function that can be used to extract any TypedObject pointer from
 * a wrapped Python object.  In this case, the Dtool_PyTypedObject is known.
 */
template<class T> INLINE bool
DtoolInstance_GetPointer(PyObject *self, T *&into, Dtool_PyTypedObject &target_class) {
  if (DtoolInstance_Check(self)) {
    if (_IS_FINAL(T)) {
      if (DtoolInstance_TYPE(self) == &target_class) {
        into = (T *)DtoolInstance_VOID_PTR(self);
      } else {
        return false;
      }
    } else {
      into = (T *)DtoolInstance_UPCAST(self, target_class);
    }
    return (into != nullptr);
  }
  into = nullptr;
  return false;
}

/**
 * Function to create a hash from a wrapped Python object.
 */
INLINE Py_hash_t DtoolInstance_HashPointer(PyObject *self) {
  if (self != nullptr && DtoolInstance_Check(self)) {
    return (Py_hash_t)(intptr_t)DtoolInstance_VOID_PTR(self);
  }
  return -1;
}

/**
 * Python 2-style comparison function that compares objects by pointer.
 */
INLINE int DtoolInstance_ComparePointers(PyObject *v1, PyObject *v2) {
  void *v1_this = DtoolInstance_Check(v1) ? DtoolInstance_VOID_PTR(v1) : nullptr;
  void *v2_this = DtoolInstance_Check(v2) ? DtoolInstance_VOID_PTR(v2) : nullptr;
  if (v1_this != nullptr && v2_this != nullptr) {
    return (v1_this > v2_this) - (v1_this < v2_this);
  } else {
    return (v1 > v2) - (v1 < v2);
  }
}

/**
 * Rich comparison function that compares objects by pointer.
 */
INLINE PyObject *DtoolInstance_RichComparePointers(PyObject *v1, PyObject *v2, int op) {
  int cmpval = DtoolInstance_ComparePointers(v1, v2);
  Py_RETURN_RICHCOMPARE(cmpval, 0, op);
}

/**
 * Utility function for assigning a PyObject pointer while managing refcounts.
 */
ALWAYS_INLINE void
Dtool_Assign_PyObject(PyObject *&ptr, PyObject *value) {
  PyObject *prev_value = ptr;
  if (prev_value != value) {
    Py_XINCREF(value);
    ptr = value;
    Py_XDECREF(prev_value);
  }
}

/**
 * Converts the enum value to a C long.
 */
INLINE long Dtool_EnumValue_AsLong(PyObject *value) {
  PyObject *val = PyObject_GetAttrString(value, "value");
  if (val != nullptr) {
    long as_long = PyLongOrInt_AS_LONG(val);
    Py_DECREF(val);
    return as_long;
  } else {
    return -1;
  }
}

/**
 * These functions wrap a pointer for a class that defines get_type_handle().
 */
template<class T> INLINE PyObject *
DTool_CreatePyInstance(const T *obj, bool memory_rules) {
  Dtool_PyTypedObject *known_class = (Dtool_PyTypedObject *)get_type_handle(T).get_python_type();
  nassertr(known_class != nullptr, nullptr);
  return DTool_CreatePyInstance((void*) obj, *known_class, memory_rules, true);
}

template<class T> INLINE PyObject *
DTool_CreatePyInstance(T *obj, bool memory_rules) {
  Dtool_PyTypedObject *known_class = (Dtool_PyTypedObject *)get_type_handle(T).get_python_type();
  nassertr(known_class != nullptr, nullptr);
  return DTool_CreatePyInstance((void*) obj, *known_class, memory_rules, false);
}

template<class T> INLINE PyObject *
DTool_CreatePyInstanceTyped(const T *obj, bool memory_rules) {
  Dtool_PyTypedObject *known_class = (Dtool_PyTypedObject *)get_type_handle(T).get_python_type();
  nassertr(known_class != nullptr, nullptr);
  return DTool_CreatePyInstanceTyped((void*) obj, *known_class, memory_rules, true, obj->get_type().get_index());
}

template<class T> INLINE PyObject *
DTool_CreatePyInstanceTyped(T *obj, bool memory_rules) {
  Dtool_PyTypedObject *known_class = (Dtool_PyTypedObject *)get_type_handle(T).get_python_type();
  nassertr(known_class != nullptr, nullptr);
  return DTool_CreatePyInstanceTyped((void*) obj, *known_class, memory_rules, false, obj->get_type().get_index());
}

/**
 * Finishes initializing the Dtool_PyInstDef.
 */
INLINE int
DTool_PyInit_Finalize(PyObject *self, void *local_this, Dtool_PyTypedObject *type, bool memory_rules, bool is_const) {
  ((Dtool_PyInstDef *)self)->_My_Type = type;
  ((Dtool_PyInstDef *)self)->_ptr_to_object = local_this;
  ((Dtool_PyInstDef *)self)->_memory_rules = memory_rules;
  ((Dtool_PyInstDef *)self)->_is_const = is_const;
  return 0;
}

/**
 * Checks that the tuple is empty.
 */
ALWAYS_INLINE bool
Dtool_CheckNoArgs(PyObject *args) {
  return PyTuple_GET_SIZE(args) == 0;
}

/**
 * Checks that the tuple is empty, and that the dict is empty or NULL.
 */
ALWAYS_INLINE bool
Dtool_CheckNoArgs(PyObject *args, PyObject *kwds) {
  return PyTuple_GET_SIZE(args) == 0 &&
    (kwds == nullptr || PyDict_GET_SIZE(kwds) == 0);
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
  // size_t is sometimes defined as unsigned long long, and we want to map
  // that to int in Python 2 so it can be returned from a __len__.
#if PY_MAJOR_VERSION >= 3
  return PyLong_FromUnsignedLongLong(value);
#else
  return (value > LONG_MAX)
    ? PyLong_FromUnsignedLongLong(value)
    : PyInt_FromLong((long)value);
#endif
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
  if (value == nullptr) {
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
  if (value == nullptr) {
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
  if (value == nullptr) {
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
  if (value == nullptr) {
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

ALWAYS_INLINE PyObject *Dtool_WrapValue(std::nullptr_t) {
  Py_INCREF(Py_None);
  return Py_None;
}

ALWAYS_INLINE PyObject *Dtool_WrapValue(PyObject *value) {
  return value;
}

ALWAYS_INLINE PyObject *Dtool_WrapValue(const vector_uchar &value) {
#if PY_MAJOR_VERSION >= 3
  return PyBytes_FromStringAndSize((char *)value.data(), (Py_ssize_t)value.size());
#else
  return PyString_FromStringAndSize((char *)value.data(), (Py_ssize_t)value.size());
#endif
}

#if PY_MAJOR_VERSION >= 0x02060000
ALWAYS_INLINE PyObject *Dtool_WrapValue(Py_buffer *value) {
  if (value == nullptr) {
    return value;
  } else {
    return PyMemoryView_FromBuffer(value);
  }
}
#endif

template<class T1, class T2>
ALWAYS_INLINE PyObject *Dtool_WrapValue(const std::pair<T1, T2> &value) {
  PyObject *tuple = PyTuple_New(2);
  PyTuple_SET_ITEM(tuple, 0, Dtool_WrapValue(value.first));
  PyTuple_SET_ITEM(tuple, 1, Dtool_WrapValue(value.second));
  return tuple;
}
