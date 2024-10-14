/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file bitArray_ext.cxx
 * @author rdb
 * @date 2020-03-21
 */

#include "bitArray_ext.h"

#ifdef HAVE_PYTHON

/**
 * Creates a BitArray from a Python long object.
 */
void Extension<BitArray>::
__init__(PyObject *init_value) {
  if (!PyLong_Check(init_value) || !PyLong_IsNonNegative(init_value)) {
    PyErr_SetString(PyExc_ValueError, "BitArray constructor requires a positive integer");
    return;
  }

  int n = _PyLong_NumBits(init_value);
  if (n > 0) {
    size_t num_words = (n + BitArray::num_bits_per_word - 1) / BitArray::num_bits_per_word;
    _this->_array.resize(num_words);
    _PyLong_AsByteArray((PyLongObject *)init_value,
      (unsigned char *)&_this->_array[0],
      num_words * sizeof(BitArray::WordType),
      1, 0
#if PY_VERSION_HEX >= 0x030d0000
      , 1 // with_exceptions
#endif
      );
  }
}

/**
 * Returns the value of the BitArray as a picklable Python object.
 *
 * We could just return a list of words.  However, different builds of Panda3D
 * may have different sizes for the WordType, so we'd also need to add code to
 * convert between different WordTypes.  Instead, we'll just encode the whole
 * array as a Python long, with infinite arrays stored as inverted longs.
 */
PyObject *Extension<BitArray>::
__getstate__() const {
  if (_this->_array.empty()) {
    return PyLong_FromLong(-_this->_highest_bits);
  }

  if (_this->_highest_bits == 0) {
    return _PyLong_FromByteArray(
      (const unsigned char *)&_this->_array[0],
      _this->_array.size() * sizeof(BitArray::WordType),
      1, 0);
  } else {
    // This is an infinite array, so we invert it to make it a finite array and
    // store it as an inverted long.
    BitArray copy(*_this);
    copy.invert_in_place();
    PyObject *state = _PyLong_FromByteArray(
      (const unsigned char *)&copy._array[0],
      copy._array.size() * sizeof(BitArray::WordType),
      1, 0);
    PyObject *inverted = PyNumber_Invert(state);
    Py_DECREF(state);
    return inverted;
  }
}

/**
 * Takes the value returned by __getstate__ and uses it to freshly initialize
 * this BitArray object.
 */
void Extension<BitArray>::
__setstate__(PyObject *state) {
  if (PyLong_IsNonNegative(state)) {
    __init__(state);
  } else {
    PyObject *inverted = PyNumber_Invert(state);
    __init__(inverted);
    Py_DECREF(inverted);
    _this->invert_in_place();
  }
}

#endif
