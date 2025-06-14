/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file doubleBitMask_ext.I
 * @author rdb
 * @date 2020-04-01
 */

/**
 * Initializes a DoubleBitMask from a Python long integer.
 */
template<class BMType>
INLINE void Extension<DoubleBitMask<BMType> >::
__init__(PyObject *init_value) {
  if (!PyLong_Check(init_value) || !PyLong_IsNonNegative(init_value)) {
    PyErr_SetString(PyExc_ValueError, "DoubleBitMask constructor requires a positive integer");
    return;
  }

  int n = _PyLong_NumBits(init_value);
  if (n > DoubleBitMask<BMType>::num_bits) {
    PyErr_SetString(PyExc_OverflowError, "value out of range for DoubleBitMask");
    return;
  }

  if (n > 0) {
    size_t num_bytes = (n + 7) / 8;
    unsigned char *bytes = (unsigned char *)alloca(num_bytes);
    _PyLong_AsByteArray((PyLongObject *)init_value, bytes, num_bytes, 1, 0
#if PY_VERSION_HEX >= 0x030d0000
      , 1 // with_exceptions
#endif
    );

    for (size_t i = 0; i < num_bytes; ++i) {
      this->_this->store(bytes[i], i * 8, 8);
    }
  }
}

/**
 * Returns true if the value is not zero.
 */
template<class BMType>
INLINE bool Extension<DoubleBitMask<BMType> >::
__bool__() const {
  return !this->_this->is_zero();
}

/**
 * Returns the value as an integer.
 */
template<class BMType>
INLINE PyObject *Extension<DoubleBitMask<BMType> >::
__int__() const {
  PyObject *result = invoke_extension(&this->_this->_lo).__int__();
  if (!this->_this->_hi.is_zero()) {
    PyObject *lo = result;
    PyObject *hi = invoke_extension(&this->_this->_hi).__int__();
#if PY_VERSION_HEX >= 0x030d0000
    PyObject *half_bits = PyLong_FromUnsignedLong(DoubleBitMask<BMType>::half_bits);
    PyObject *shifted = PyNumber_Lshift(hi, half_bits);
    Py_DECREF(half_bits);
#else
    PyObject *shifted = _PyLong_Lshift(hi, DoubleBitMask<BMType>::half_bits);
#endif
    Py_DECREF(hi);
    result = PyNumber_Or(shifted, lo);
    Py_DECREF(shifted);
    Py_DECREF(lo);
  }
  return result;
}

/**
 * This special Python method is implemented to provide support for the pickle
 * module.
 */
template<class BMType>
INLINE PyObject *Extension<DoubleBitMask<BMType> >::
__reduce__(PyObject *self) const {
  // We should return at least a 2-tuple, (Class, (args)): the necessary class
  // object whose constructor we should call (e.g.  this), and the arguments
  // necessary to reconstruct this object.
  return Py_BuildValue("(O(N))", Py_TYPE(self), __int__());
}
