// Filename: transformState_ext.cxx
// Created by:  CFSworks (30Mar14)
//
////////////////////////////////////////////////////////////////////
//
// PANDA 3D SOFTWARE
// Copyright (c) Carnegie Mellon University.  All rights reserved.
//
// All use of this software is subject to the terms of the revised BSD
// license.  You should have received a copy of this license along
// with this source code in a file named "LICENSE."
//
////////////////////////////////////////////////////////////////////

#include "transformState_ext.h"

#ifdef HAVE_PYTHON

////////////////////////////////////////////////////////////////////
//     Function: Extension<TransformState>::get_composition_cache
//       Access: Published
//  Description: Returns a list of 2-tuples that represents the
//               composition cache.  For each tuple in the list, the
//               first element is the source transform, and the second
//               is the result transform.  If both are None, there is
//               no entry in the cache at that slot.
//
//               In general, a->compose(source) == result.
//
//               This has no practical value other than for examining
//               the cache for performance analysis.
////////////////////////////////////////////////////////////////////
PyObject *Extension<TransformState>::
get_composition_cache() const {
  IMPORT_THIS struct Dtool_PyTypedObject Dtool_TransformState;
  LightReMutexHolder holder(*_this->_states_lock);

  size_t num_states = _this->_composition_cache.get_num_entries();
  PyObject *list = PyList_New(num_states);
  size_t i = 0;

  int size = _this->_composition_cache.get_size();
  for (int si = 0; si < size; ++si) {
    if (!_this->_composition_cache.has_element(si)) {
      continue;
    }

    PyObject *tuple = PyTuple_New(2);
    PyObject *a, *b;

    const TransformState *source = _this->_composition_cache.get_key(si);
    if (source == (TransformState *)NULL) {
      a = Py_None;
      Py_INCREF(a);
    } else {
      source->ref();
      a = DTool_CreatePyInstanceTyped((void *)source, Dtool_TransformState, 
                                      true, true, source->get_type_index());
    }
    const TransformState *result = _this->_composition_cache.get_data(si)._result;
    if (result == (TransformState *)NULL) {
      b = Py_None;
      Py_INCREF(b);
    } else {
      result->ref();
      b = DTool_CreatePyInstanceTyped((void *)result, Dtool_TransformState, 
                                      true, true, result->get_type_index());
    }

    PyTuple_SET_ITEM(tuple, 0, a);
    PyTuple_SET_ITEM(tuple, 1, b);

    nassertr(i < num_states, list);
    PyList_SET_ITEM(list, i, tuple);
    ++i;
  }
  nassertr(i == num_states, list);
  return list;
}

////////////////////////////////////////////////////////////////////
//     Function: Extension<TransformState>::get_invert_composition_cache
//       Access: Published
//  Description: Returns a list of 2-tuples that represents the
//               invert_composition cache.  For each tuple in the list, the
//               first element is the source transform, and the second
//               is the result transform.  If both are None, there is
//               no entry in the cache at that slot.
//
//               In general, a->invert_compose(source) == result.
//
//               This has no practical value other than for examining
//               the cache for performance analysis.
////////////////////////////////////////////////////////////////////
PyObject *Extension<TransformState>::
get_invert_composition_cache() const {
  IMPORT_THIS struct Dtool_PyTypedObject Dtool_TransformState;
  LightReMutexHolder holder(*_this->_states_lock);

  size_t num_states = _this->_invert_composition_cache.get_num_entries();
  PyObject *list = PyList_New(num_states);
  size_t i = 0;

  int size = _this->_invert_composition_cache.get_size();
  for (int si = 0; si < size; ++si) {
    if (!_this->_invert_composition_cache.has_element(si)) {
      continue;
    }

    PyObject *tuple = PyTuple_New(2);
    PyObject *a, *b;

    const TransformState *source = _this->_invert_composition_cache.get_key(si);
    if (source == (TransformState *)NULL) {
      a = Py_None;
      Py_INCREF(a);
    } else {
      source->ref();
      a = DTool_CreatePyInstanceTyped((void *)source, Dtool_TransformState, 
                                      true, true, source->get_type_index());
    }
    const TransformState *result = _this->_invert_composition_cache.get_data(si)._result;
    if (result == (TransformState *)NULL) {
      b = Py_None;
      Py_INCREF(b);
    } else {
      result->ref();
      b = DTool_CreatePyInstanceTyped((void *)result, Dtool_TransformState, 
                                      true, true, result->get_type_index());
    }

    PyTuple_SET_ITEM(tuple, 0, a);
    PyTuple_SET_ITEM(tuple, 1, b);

    nassertr(i < num_states, list);
    PyList_SET_ITEM(list, i, tuple);
    ++i;
  }
  nassertr(i == num_states, list);
  return list;
}

////////////////////////////////////////////////////////////////////
//     Function: Extension<TransformState>::get_states
//       Access: Published, Static
//  Description: Returns a list of all of the TransformState objects
//               in the state cache.  The order of elements in this
//               cache is arbitrary.
////////////////////////////////////////////////////////////////////
PyObject *Extension<TransformState>::
get_states() {
  IMPORT_THIS struct Dtool_PyTypedObject Dtool_TransformState;
  if (TransformState::_states == (TransformState::States *)NULL) {
    return PyList_New(0);
  }
  LightReMutexHolder holder(*TransformState::_states_lock);

  size_t num_states = TransformState::_states->get_num_entries();
  PyObject *list = PyList_New(num_states);
  size_t i = 0;

  int size = TransformState::_states->get_size();
  for (int si = 0; si < size; ++si) {
    if (!TransformState::_states->has_element(si)) {
      continue;
    }
    const TransformState *state = TransformState::_states->get_key(si);
    state->ref();
    PyObject *a = 
      DTool_CreatePyInstanceTyped((void *)state, Dtool_TransformState, 
                                  true, true, state->get_type_index());
    nassertr(i < num_states, list);
    PyList_SET_ITEM(list, i, a);
    ++i;
  }
  nassertr(i == num_states, list);
  return list;
}

////////////////////////////////////////////////////////////////////
//     Function: Extension<TransformState>::get_unused_states
//       Access: Published, Static
//  Description: Returns a list of all of the "unused" TransformState
//               objects in the state cache.  See
//               get_num_unused_states().
////////////////////////////////////////////////////////////////////
PyObject *Extension<TransformState>::
get_unused_states() {
  IMPORT_THIS struct Dtool_PyTypedObject Dtool_TransformState;
  if (TransformState::_states == (TransformState::States *)NULL) {
    return PyList_New(0);
  }
  LightReMutexHolder holder(*TransformState::_states_lock);

  PyObject *list = PyList_New(0);
  int size = TransformState::_states->get_size();
  for (int si = 0; si < size; ++si) {
    if (!TransformState::_states->has_element(si)) {
      continue;
    }
    const TransformState *state = TransformState::_states->get_key(si);
    if (state->get_cache_ref_count() == state->get_ref_count()) {
      state->ref();
      PyObject *a = 
        DTool_CreatePyInstanceTyped((void *)state, Dtool_TransformState, 
                                    true, true, state->get_type_index());
      PyList_Append(list, a);
      Py_DECREF(a);
    }
  }
  return list;
}

#endif  // HAVE_PYTHON
