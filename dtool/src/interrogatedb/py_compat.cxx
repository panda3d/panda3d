/**
 * @file py_compat.cxx
 * @author rdb
 * @date 2017-12-03
 */

#include "py_compat.h"
#include "py_panda.h"

#ifdef HAVE_PYTHON

#if PY_MAJOR_VERSION < 3
/**
 * Given a long or int, returns a size_t, or raises an OverflowError if it is
 * out of range.
 */
size_t PyLongOrInt_AsSize_t(PyObject *vv) {
  if (PyInt_Check(vv)) {
    long value = PyInt_AS_LONG(vv);
    if (value < 0) {
      PyErr_SetString(PyExc_OverflowError,
                      "can't convert negative value to size_t");
      return (size_t)-1;
    }
    return (size_t)value;
  }

  if (!PyLong_Check(vv)) {
    Dtool_Raise_TypeError("a long or int was expected");
    return (size_t)-1;
  }

  size_t bytes;
  int one = 1;
  int res = _PyLong_AsByteArray((PyLongObject *)vv, (unsigned char *)&bytes,
                                SIZEOF_SIZE_T, (int)*(unsigned char*)&one, 0);

  if (res < 0) {
    return (size_t)res;
  } else {
    return bytes;
  }
}
#endif

#if PY_VERSION_HEX < 0x03090000
/**
 * Most efficient way to call a function without any arguments.
 */
PyObject *PyObject_CallNoArgs(PyObject *func) {
#if PY_VERSION_HEX >= 0x03080000
  return _PyObject_Vectorcall(func, nullptr, 0, nullptr);
#elif PY_VERSION_HEX >= 0x03070000
  return _PyObject_FastCallDict(func, nullptr, 0, nullptr);
#elif PY_VERSION_HEX >= 0x03060000
  return _PyObject_FastCall(func, nullptr, 0);
#else
  static PyObject *empty_tuple = PyTuple_New(0);
  return PyObject_Call(func, empty_tuple, nullptr);
#endif
}
#endif

#endif  // HAVE_PYTHON
