/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file pandaNode_ext.cxx
 * @author CFSworks
 * @date 2014-03-30
 */

#include "pandaNode_ext.h"

#ifdef HAVE_PYTHON

/**
 * A special Python method that is invoked by copy.copy(node).  Unlike the
 * PandaNode copy constructor, which creates a new node without children, this
 * shares child pointers (essentially making every child an instance).  This
 * is intended to simulate the behavior of copy.copy() for other objects.
 */
PT(PandaNode) Extension<PandaNode>::
__copy__() const {
  Thread *current_thread = Thread::get_current_thread();

  PT(PandaNode) node_dupe = _this->make_copy();

  PandaNode::Children children = _this->get_children(current_thread);
  int num_children = children.get_num_children();

  for (int i = 0; i < num_children; ++i) {
    node_dupe->add_child(children.get_child(i), children.get_child_sort(i));
  }

  return node_dupe;
}

/**
 * A special Python method that is invoked by copy.deepcopy(node).  This calls
 * copy_subgraph() unless the node is already present in the provided
 * dictionary.
 */
PyObject *Extension<PandaNode>::
__deepcopy__(PyObject *self, PyObject *memo) const {
  extern struct Dtool_PyTypedObject Dtool_PandaNode;

  // Borrowed reference.
  PyObject *dupe = PyDict_GetItem(memo, self);
  if (dupe != nullptr) {
    // Already in the memo dictionary.
    Py_INCREF(dupe);
    return dupe;
  }

  PT(PandaNode) node_dupe = _this->copy_subgraph();

  // DTool_CreatePyInstanceTyped() steals a C++ reference.
  node_dupe->ref();
  dupe = DTool_CreatePyInstanceTyped
    ((void *)node_dupe.p(), Dtool_PandaNode, true, false,
     node_dupe->get_type_index());

  if (PyDict_SetItem(memo, self, dupe) != 0) {
    Py_DECREF(dupe);
    return nullptr;
  }

  return dupe;
}

/**
 * This variant on get_tag_keys returns a Python tuple of strings keys.
 */
PyObject *Extension<PandaNode>::
get_tag_keys() const {
  vector_string keys;
  _this->get_tag_keys(keys);

  PyObject *result = PyTuple_New(keys.size());
  for (size_t i = 0; i < keys.size(); ++i) {
    PyTuple_SET_ITEM(result, i, Dtool_WrapValue(keys[i]));
  }

  return result;
}

/**
 * Returns a dictionary of Python tags that is associated with the current
 * node.  This is similar to the regular tags, except this can store any
 * Python object instead of just a string.  However, the Python object is not
 * recorded to a bam file.
 */
PyObject *Extension<PandaNode>::
get_python_tags() {
  PyObject *dict = do_get_python_tags();
  Py_INCREF(dict);
  return dict;
}

/**
 * Associates an arbitrary Python object with a user-defined key which is
 * stored on the node.  This is similar to set_tag(), except it can store any
 * Python object instead of just a string.  However, the Python object is not
 * recorded to a bam file.
 *
 * Each unique key stores a different string value.  There is no effective
 * limit on the number of different keys that may be stored or on the length
 * of any one key's value.
 */
int Extension<PandaNode>::
set_python_tag(PyObject *key, PyObject *value) {
  return PyDict_SetItem(do_get_python_tags(), key, value);
}

/**
 * Retrieves the Python object that was previously set on this node for the
 * particular key, if any.  If no value has been previously set, returns None.
 */
PyObject *Extension<PandaNode>::
get_python_tag(PyObject *key) const {
  if (_this->_python_tag_data == nullptr) {
    Py_INCREF(Py_None);
    return Py_None;
  }

  PyObject *dict = ((PythonTagDataImpl *)_this->_python_tag_data.p())->_dict;
  PyObject *value = PyDict_GetItem(dict, key);
  if (value == nullptr) {
    value = Py_None;
  }
  // PyDict_GetItem returns a borrowed reference.
  Py_INCREF(value);
  return value;
}

/**
 * Returns true if a Python object has been defined on this node for the
 * particular key (even if that object is None), or false if no object has
 * been set.
 */
bool Extension<PandaNode>::
has_python_tag(PyObject *key) const {
  if (_this->_python_tag_data == nullptr) {
    return false;
  }

  PyObject *dict = ((PythonTagDataImpl *)_this->_python_tag_data.p())->_dict;
  return (PyDict_GetItem(dict, key) != nullptr);
}

/**
 * Removes the Python object defined for this key on this particular node.
 * After a call to clear_python_tag(), has_python_tag() will return false for
 * the indicated key.
 */
void Extension<PandaNode>::
clear_python_tag(PyObject *key) {
  if (_this->_python_tag_data == nullptr) {
    return;
  }

  PyObject *dict = do_get_python_tags();
  if (PyDict_GetItem(dict, key) != nullptr) {
    PyDict_DelItem(dict, key);
  }
}

/**
 * This variant on get_python_tag_keys returns a Python list of strings.
 */
PyObject *Extension<PandaNode>::
get_python_tag_keys() const {
  if (_this->_python_tag_data == nullptr) {
    return PyTuple_New(0);
  }

  PyObject *dict = ((PythonTagDataImpl *)_this->_python_tag_data.p())->_dict;
  return PyDict_Keys(dict);
}

/**
 * Called by Python to implement cycle detection.
 */
int Extension<PandaNode>::
__traverse__(visitproc visit, void *arg) {
  // To fully implement cycle breaking, we also have to recurse into all of
  // the node's children.  However, this seems like it would be potentially
  // quite expensive, so I'd rather not do it unless we had some optimization
  // that would allow us to quickly find out whether there are children with
  // Python tags.
  if (_this->_python_tag_data != nullptr) {
    Py_VISIT(((PythonTagDataImpl *)_this->_python_tag_data.p())->_dict);
  }
  return 0;
}

/**
 * Same as get_python_tags, without incrementing the reference count.
 */
PyObject *Extension<PandaNode>::
do_get_python_tags() {
  if (_this->_python_tag_data == nullptr) {
    _this->_python_tag_data = new PythonTagDataImpl;

  } else if (_this->_python_tag_data->get_ref_count() > 1) {
    // Copy-on-write.
    _this->_python_tag_data = new PythonTagDataImpl(*(PythonTagDataImpl *)_this->_python_tag_data.p());
  }
  return ((PythonTagDataImpl *)_this->_python_tag_data.p())->_dict;
}

/**
 * Destroys the tags associated with the node.
 */
Extension<PandaNode>::PythonTagDataImpl::
~PythonTagDataImpl() {
#if defined(HAVE_THREADS) && !defined(SIMPLE_THREADS)
  // This might happen at any time, so be sure the Python state is ready for
  // it.
  PyGILState_STATE gstate;
  gstate = PyGILState_Ensure();
#endif

  Py_CLEAR(_dict);

#if defined(HAVE_THREADS) && !defined(SIMPLE_THREADS)
  PyGILState_Release(gstate);
#endif
}

#endif  // HAVE_PYTHON
