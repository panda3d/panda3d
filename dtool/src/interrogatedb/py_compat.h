/**
 * @file py_compat.h
 * @author rdb
 * @date 2017-12-02
 */

#ifndef PY_COMPAT_H
#define PY_COMPAT_H

#include "dtoolbase.h"

#ifdef HAVE_PYTHON

// The contents of this file were originally part of py_panda.h.  It
// specifically contains polyfills that are required to maintain compatibility
// with Python 2 and older versions of Python 3.

// These compatibility hacks are sorted by Python version that removes the
// need for the respective hack.

#ifdef _POSIX_C_SOURCE
#  undef _POSIX_C_SOURCE
#endif

#ifdef _XOPEN_SOURCE
#  undef _XOPEN_SOURCE
#endif

// See PEP 353
#define PY_SSIZE_T_CLEAN 1

#include <Python.h>

#ifndef LINK_ALL_STATIC
#  define EXPCL_PYPANDA
#elif defined(__GNUC__)
#  define EXPCL_PYPANDA __attribute__((weak))
#else
#  define EXPCL_PYPANDA extern inline
#endif

/* Python 2.4 */

// 2.4 macros which aren't available in 2.3
#ifndef Py_RETURN_NONE
#  define Py_RETURN_NONE return Py_INCREF(Py_None), Py_None
#endif

#ifndef Py_RETURN_TRUE
#  define Py_RETURN_TRUE return Py_INCREF(Py_True), Py_True
#endif

#ifndef Py_RETURN_FALSE
#  define Py_RETURN_FALSE return Py_INCREF(Py_False), Py_False
#endif

/* Python 2.5 */

// Prior to Python 2.5, we didn't have Py_ssize_t.
#if PY_VERSION_HEX < 0x02050000
typedef int Py_ssize_t;
#  define PyInt_FromSsize_t PyInt_FromLong
#  define PyInt_AsSsize_t PyInt_AsLong
#endif

/* Python 2.6 */

#ifndef Py_TYPE
#  define Py_TYPE(ob) (((PyObject*)(ob))->ob_type)
#endif

/* Python 2.7, 3.1 */

#ifndef PyVarObject_HEAD_INIT
  #define PyVarObject_HEAD_INIT(type, size) \
    PyObject_HEAD_INIT(type) size,
#endif

/* Python 2.7, 3.2 */

#if PY_VERSION_HEX < 0x03020000
#  define PyErr_NewExceptionWithDoc(name, doc, base, dict) \
          PyErr_NewException(name, base, dict)
#endif

/* Python 3.0 */

// Always on in Python 3
#ifndef Py_TPFLAGS_CHECKTYPES
#  define Py_TPFLAGS_CHECKTYPES 0
#endif

// Macros for writing code that will compile in both versions.
#if PY_MAJOR_VERSION >= 3
#  define nb_nonzero nb_bool
#  define nb_divide nb_true_divide
#  define nb_inplace_divide nb_inplace_true_divide

#  define PyLongOrInt_Check(x) PyLong_Check(x)
#  define PyLongOrInt_AS_LONG PyLong_AS_LONG
#  define PyInt_Check PyLong_Check
#  define PyInt_AsLong PyLong_AsLong
#  define PyInt_AS_LONG PyLong_AS_LONG
#  define PyLongOrInt_AsSize_t PyLong_AsSize_t
#else
#  define PyLongOrInt_Check(x) (PyInt_Check(x) || PyLong_Check(x))
// PyInt_FromSize_t automatically picks the right type.
#  define PyLongOrInt_AS_LONG PyInt_AsLong

EXPCL_PYPANDA size_t PyLongOrInt_AsSize_t(PyObject *);
#endif

// Which character to use in PyArg_ParseTuple et al for a byte string.
#if PY_MAJOR_VERSION >= 3
#  define FMTCHAR_BYTES "y"
#else
#  define FMTCHAR_BYTES "s"
#endif

/* Python 3.2 */

#if PY_VERSION_HEX < 0x03020000
typedef long Py_hash_t;
#endif

/* Python 3.3 */

#if PY_MAJOR_VERSION >= 3
// Python 3 versions before 3.3.3 defined this incorrectly.
#  undef _PyErr_OCCURRED
#  define _PyErr_OCCURRED() (PyThreadState_GET()->curexc_type)

// Python versions before 3.3 did not define this.
#  if PY_VERSION_HEX < 0x03030000
#    define PyUnicode_AsUTF8 _PyUnicode_AsString
#    define PyUnicode_AsUTF8AndSize _PyUnicode_AsStringAndSize
#  endif
#endif

/* Python 3.6 */

#if PY_VERSION_HEX < 0x03080000 && !defined(_PyObject_CallNoArg)
INLINE PyObject *_PyObject_CallNoArg(PyObject *func) {
  static PyTupleObject empty_tuple = {PyVarObject_HEAD_INIT(nullptr, 0)};
#ifdef Py_TRACE_REFS
  _Py_AddToAllObjects((PyObject *)&empty_tuple, 0);
#endif
  return PyObject_Call(func, (PyObject *)&empty_tuple, nullptr);
}
#  define _PyObject_CallNoArg _PyObject_CallNoArg
#endif

#ifndef _PyObject_FastCall
INLINE PyObject *_PyObject_FastCall(PyObject *func, PyObject **args, Py_ssize_t nargs) {
  PyObject *tuple = PyTuple_New(nargs);
  for (Py_ssize_t i = 0; i < nargs; ++i) {
    PyTuple_SET_ITEM(tuple, i, args[i]);
    Py_INCREF(args[i]);
  }
  PyObject *result = PyObject_Call(func, tuple, nullptr);
  Py_DECREF(tuple);
  return result;
}
#  define _PyObject_FastCall _PyObject_FastCall
#endif

// Python versions before 3.6 didn't require longlong support to be enabled.
#ifndef HAVE_LONG_LONG
#  define PyLong_FromLongLong(x) PyLong_FromLong((long) (x))
#  define PyLong_FromUnsignedLongLong(x) PyLong_FromUnsignedLong((unsigned long) (x))
#  define PyLong_AsLongLong(x) PyLong_AsLong(x)
#  define PyLong_AsUnsignedLongLong(x) PyLong_AsUnsignedLong(x)
#  define PyLong_AsUnsignedLongLongMask(x) PyLong_AsUnsignedLongMask(x)
#  define PyLong_AsLongLongAndOverflow(x) PyLong_AsLongAndOverflow(x)
#endif

/* Python 3.7 */

#ifndef PyDict_GET_SIZE
#  define PyDict_GET_SIZE(mp) (((PyDictObject *)mp)->ma_used)
#endif

#ifndef Py_RETURN_RICHCOMPARE
#  define Py_RETURN_RICHCOMPARE(val1, val2, op)                         \
  do {                                                                  \
    switch (op) {                                                       \
    NODEFAULT                                                           \
    case Py_EQ: if ((val1) == (val2)) Py_RETURN_TRUE; Py_RETURN_FALSE;  \
    case Py_NE: if ((val1) != (val2)) Py_RETURN_TRUE; Py_RETURN_FALSE;  \
    case Py_LT: if ((val1) < (val2)) Py_RETURN_TRUE; Py_RETURN_FALSE;   \
    case Py_GT: if ((val1) > (val2)) Py_RETURN_TRUE; Py_RETURN_FALSE;   \
    case Py_LE: if ((val1) <= (val2)) Py_RETURN_TRUE; Py_RETURN_FALSE;  \
    case Py_GE: if ((val1) >= (val2)) Py_RETURN_TRUE; Py_RETURN_FALSE;  \
    }                                                                   \
  } while (0)
#endif

/* Other Python implementations */

// _PyErr_OCCURRED is an undocumented macro version of PyErr_Occurred.
// Some implementations of the CPython API (e.g. PyPy's cpyext) do not define
// it, so in these cases we just silently fall back to PyErr_Occurred.
#ifndef _PyErr_OCCURRED
#  define _PyErr_OCCURRED() PyErr_Occurred()
#endif

#endif  // HAVE_PYTHON

#endif  // PY_COMPAT_H
