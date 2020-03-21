/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file sparseArray_ext.cxx
 * @author rdb
 * @date 2020-03-21
 */

#include "sparseArray_ext.h"

#ifdef HAVE_PYTHON

/**
 * Returns the value of the SparseArray as a picklable Python object.
 *
 * We store this as a tuple of integers.  The first number indicates the first
 * bit that is set to 1, the second number indicates the next bit that is set to
 * 0, the third number indicates the next bit that is set to 1, etc.  Using this
 * method, we store an uneven number of integers for an inverted SparseArray,
 * and an even number for a regular SparseArray.
 *
 * This table demonstrates the three different cases:
 *
 *  | range      | pickled     | SparseArray |
 *  |------------|-------------|-------------|
 *  | 0-2, 4-8   | 0, 2, 4, 8  | 0-2, 4-8    |
 *  | 0-2, 4-... | 0, 2, 4     | ~ 2-4       |
 *  | 2-4, 6-... | 2, 4, 6     | ~ 0-2, 4-6  |
 *
 */
PyObject *Extension<SparseArray>::
__getstate__() const {
  PyObject *state;
  Py_ssize_t index = 0;
  size_t sri = 0;
  size_t num_ranges = _this->get_num_subranges();

  if (!_this->is_inverse()) {
    state = PyTuple_New(num_ranges * 2);
  }
  else if (num_ranges > 0 && _this->get_subrange_begin(0) == 0) {
    // Prevent adding a useless 0-0 range at the beginning.
    state = PyTuple_New(num_ranges * 2 - 1);
    PyTuple_SET_ITEM(state, index++, Dtool_WrapValue(_this->get_subrange_end(sri++)));
  }
  else {
    state = PyTuple_New(num_ranges * 2 + 1);
    PyTuple_SET_ITEM(state, index++, Dtool_WrapValue(0));
  }

  for (; sri < num_ranges; ++sri) {
    PyTuple_SET_ITEM(state, index++, Dtool_WrapValue(_this->get_subrange_begin(sri)));
    PyTuple_SET_ITEM(state, index++, Dtool_WrapValue(_this->get_subrange_end(sri)));
  }
  return state;
}

/**
 * Takes the tuple returned by __getstate__ and uses it to freshly initialize
 * this SparseArray object.
 */
void Extension<SparseArray>::
__setstate__(PyObject *state) {
  _this->clear();

  Py_ssize_t i = 0;
  Py_ssize_t len = PyTuple_GET_SIZE(state);
  if (len % 2 != 0) {
    // An uneven number of elements indicates an open final range.
    // This translates to an inverted range in SparseArray's representation.
    _this->invert_in_place();
    long first = PyLongOrInt_AS_LONG(PyTuple_GET_ITEM(state, 0));
    if (first != 0) {
      // It doesn't start at 0, so we have to first disable this range.
      _this->do_add_range(0, (int)first);
    }
    ++i;
  }

  for (; i < len; i += 2) {
    _this->do_add_range(
      PyLongOrInt_AS_LONG(PyTuple_GET_ITEM(state, i)),
      PyLongOrInt_AS_LONG(PyTuple_GET_ITEM(state, i + 1))
    );
  }
}

#endif
