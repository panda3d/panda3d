// Filename: renderState_ext.cxx
// Created by:  CFSworks (31Mar14)
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

#include "renderState_ext.h"

#ifdef HAVE_PYTHON

////////////////////////////////////////////////////////////////////
//     Function: Extension<RenderState>::get_composition_cache
//       Access: Published
//  Description: Returns a list of 2-tuples that represents the
//               composition cache.  For each tuple in the list, the
//               first element is the source render, and the second
//               is the result render.  If both are None, there is
//               no entry in the cache at that slot.
//
//               In general, a->compose(source) == result.
//
//               This has no practical value other than for examining
//               the cache for performance analysis.
////////////////////////////////////////////////////////////////////
PyObject *Extension<RenderState>::
get_composition_cache() const {
  IMPORT_THIS struct Dtool_PyTypedObject Dtool_RenderState;
  LightReMutexHolder holder(*RenderState::_states_lock);
  size_t cache_size = _this->_composition_cache.get_size();
  PyObject *list = PyList_New(cache_size);

  for (size_t i = 0; i < cache_size; ++i) {
    PyObject *tuple = PyTuple_New(2);
    PyObject *a, *b;
    if (!_this->_composition_cache.has_element(i)) {
      a = Py_None;
      Py_INCREF(a);
      b = Py_None;
      Py_INCREF(b);
    } else {
      const RenderState *source = _this->_composition_cache.get_key(i);
      if (source == (RenderState *)NULL) {
        a = Py_None;
        Py_INCREF(a);
      } else {
        source->ref();
        a = DTool_CreatePyInstanceTyped((void *)source, Dtool_RenderState, 
                                        true, true, source->get_type_index());
      }
      const RenderState *result = _this->_composition_cache.get_data(i)._result;
      if (result == (RenderState *)NULL) {
        b = Py_None;
        Py_INCREF(b);
      } else {
        result->ref();
        b = DTool_CreatePyInstanceTyped((void *)result, Dtool_RenderState, 
                                        true, true, result->get_type_index());
      }
    }
    PyTuple_SET_ITEM(tuple, 0, a);
    PyTuple_SET_ITEM(tuple, 1, b);

    PyList_SET_ITEM(list, i, tuple);
  }

  return list;
}

////////////////////////////////////////////////////////////////////
//     Function: Extension<RenderState>::get_invert_composition_cache
//       Access: Published
//  Description: Returns a list of 2-tuples that represents the
//               invert_composition cache.  For each tuple in the list, the
//               first element is the source render, and the second
//               is the result render.  If both are None, there is
//               no entry in the cache at that slot.
//
//               In general, a->invert_compose(source) == result.
//
//               This has no practical value other than for examining
//               the cache for performance analysis.
////////////////////////////////////////////////////////////////////
PyObject *Extension<RenderState>::
get_invert_composition_cache() const {
  IMPORT_THIS struct Dtool_PyTypedObject Dtool_RenderState;
  LightReMutexHolder holder(*RenderState::_states_lock);
  size_t cache_size = _this->_invert_composition_cache.get_size();
  PyObject *list = PyList_New(cache_size);

  for (size_t i = 0; i < cache_size; ++i) {
    PyObject *tuple = PyTuple_New(2);
    PyObject *a, *b;
    if (!_this->_invert_composition_cache.has_element(i)) {
      a = Py_None;
      Py_INCREF(a);
      b = Py_None;
      Py_INCREF(b);
    } else {
      const RenderState *source = _this->_invert_composition_cache.get_key(i);
      if (source == (RenderState *)NULL) {
        a = Py_None;
        Py_INCREF(a);
      } else {
        source->ref();
        a = DTool_CreatePyInstanceTyped((void *)source, Dtool_RenderState, 
                                        true, true, source->get_type_index());
      }
      const RenderState *result = _this->_invert_composition_cache.get_data(i)._result;
      if (result == (RenderState *)NULL) {
        b = Py_None;
        Py_INCREF(b);
      } else {
        result->ref();
        b = DTool_CreatePyInstanceTyped((void *)result, Dtool_RenderState, 
                                        true, true, result->get_type_index());
      }
    }
    PyTuple_SET_ITEM(tuple, 0, a);
    PyTuple_SET_ITEM(tuple, 1, b);

    PyList_SET_ITEM(list, i, tuple);
  }

  return list;
}

////////////////////////////////////////////////////////////////////
//     Function: Extension<RenderState>::get_states
//       Access: Published, Static
//  Description: Returns a list of all of the RenderState objects
//               in the state cache.  The order of elements in this
//               cache is arbitrary.
////////////////////////////////////////////////////////////////////
PyObject *Extension<RenderState>::
get_states() {
  IMPORT_THIS struct Dtool_PyTypedObject Dtool_RenderState;
  if (RenderState::_states == (RenderState::States *)NULL) {
    return PyList_New(0);
  }
  LightReMutexHolder holder(*RenderState::_states_lock);

  size_t num_states = RenderState::_states->get_num_entries();
  PyObject *list = PyList_New(num_states);
  size_t i = 0;

  int size = RenderState::_states->get_size();
  for (int si = 0; si < size; ++si) {
    if (!RenderState::_states->has_element(si)) {
      continue;
    }
    const RenderState *state = RenderState::_states->get_key(si);
    state->ref();
    PyObject *a = 
      DTool_CreatePyInstanceTyped((void *)state, Dtool_RenderState, 
                                  true, true, state->get_type_index());
    nassertr(i < num_states, list);
    PyList_SET_ITEM(list, i, a);
    ++i;
  }
  nassertr(i == num_states, list);
  return list;
}



#endif  // HAVE_PYTHON
