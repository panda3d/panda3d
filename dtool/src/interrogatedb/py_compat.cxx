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
#endif  // PY_MAJOR_VERSION < 3

#if PY_VERSION_HEX < 0x03070000
/**
 * Similar to PyArg_UnpackTuple, but takes an argument stack instead.
 */
int _PyArg_UnpackStack(PyObject *const *args, Py_ssize_t nargs, const char *name,
                       Py_ssize_t min, Py_ssize_t max, ...) {
  va_list vargs;
#ifdef HAVE_STDARG_PROTOTYPES
  va_start(vargs, max);
#else
  va_start(vargs);
#endif

  if (nargs >= min && nargs <= max) {
    for (Py_ssize_t i = 0; i < nargs; i++) {
      PyObject **ptr = va_arg(vargs, PyObject **);
      *ptr = args[i];
    }
    va_end(vargs);
    return 1;

  } else if (nargs < min) {
    PyErr_Format(PyExc_TypeError, "%.200s expected %s%zd arguments, got %zd",
                 name, (min == max ? "" : "at least "), min, nargs);
    va_end(vargs);
    return 0;

  } else {
    PyErr_Format(PyExc_TypeError, "%.200s expected %s%zd arguments, got %zd",
                 name, (min == max ? "" : "at most "), max, nargs);
    va_end(vargs);
    return 0;
  }
}
#endif  // PY_VERSION_HEX < 0x03070000

#endif  // HAVE_PYTHON
