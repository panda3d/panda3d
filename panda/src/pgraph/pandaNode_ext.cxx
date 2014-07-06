// Filename: pandaNode_ext.cxx
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

#include "pandaNode_ext.h"

#ifdef HAVE_PYTHON

////////////////////////////////////////////////////////////////////
//     Function: Extension<PandaNode>::__copy__
//       Access: Published
//  Description: A special Python method that is invoked by
//               copy.copy(node).  Unlike the PandaNode copy
//               constructor, which creates a new node without
//               children, this shares child pointers (essentially
//               making every child an instance).  This is intended to
//               simulate the behavior of copy.copy() for other
//               objects.
////////////////////////////////////////////////////////////////////
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

////////////////////////////////////////////////////////////////////
//     Function: Extension<PandaNode>::__deepcopy__
//       Access: Published
//  Description: A special Python method that is invoked by
//               copy.deepcopy(node).  This calls copy_subgraph()
//               unless the node is already present in the provided
//               dictionary.
////////////////////////////////////////////////////////////////////
PyObject *Extension<PandaNode>::
__deepcopy__(PyObject *self, PyObject *memo) const {
  IMPORT_THIS struct Dtool_PyTypedObject Dtool_PandaNode;

  // Borrowed reference.
  PyObject *dupe = PyDict_GetItem(memo, self);
  if (dupe != NULL) {
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
    return NULL;
  }

  return dupe;
}

////////////////////////////////////////////////////////////////////
//     Function: Extension<PandaNode>::set_python_tag
//       Access: Published
//  Description: Associates an arbitrary Python object with a
//               user-defined key which is stored on the node.  This
//               is similar to set_tag(), except it can store any
//               Python object instead of just a string.  However, the
//               Python object is not recorded to a bam file.
//
//               Each unique key stores a different string value.
//               There is no effective limit on the number of
//               different keys that may be stored or on the length of
//               any one key's value.
////////////////////////////////////////////////////////////////////
void Extension<PandaNode>::
set_python_tag(const string &key, PyObject *value) {
  Thread *current_thread = Thread::get_current_thread();
  int pipeline_stage = current_thread->get_pipeline_stage();
  nassertv(pipeline_stage == 0);

  PandaNode::CDWriter cdata(_this->_cycler);
  Py_XINCREF(value);

  pair<PandaNode::PythonTagData::iterator, bool> result;
  result = cdata->_python_tag_data.insert(PandaNode::PythonTagData::value_type(key, value));

  if (!result.second) {
    // The insert was unsuccessful; that means the key was already
    // present in the map.  In this case, we should decrement the
    // original value's reference count and replace it with the new
    // object.
    PandaNode::PythonTagData::iterator ti = result.first;
    PyObject *old_value = (*ti).second;
    Py_XDECREF(old_value);
    (*ti).second = value;
  }

  // Even though the python tag isn't recorded in the bam stream?
  _this->mark_bam_modified();
}

////////////////////////////////////////////////////////////////////
//     Function: Extension<PandaNode>::get_python_tag
//       Access: Published
//  Description: Retrieves the Python object that was previously
//               set on this node for the particular key, if any.  If
//               no value has been previously set, returns None.
////////////////////////////////////////////////////////////////////
PyObject *Extension<PandaNode>::
get_python_tag(const string &key) const {
  PandaNode::CDReader cdata(_this->_cycler);
  PandaNode::PythonTagData::const_iterator ti;
  ti = cdata->_python_tag_data.find(key);
  if (ti != cdata->_python_tag_data.end()) {
    PyObject *result = (*ti).second;
    Py_XINCREF(result);
    return result;
  }
  Py_INCREF(Py_None);
  return Py_None;
}

////////////////////////////////////////////////////////////////////
//     Function: Extension<PandaNode>::has_python_tag
//       Access: Published
//  Description: Returns true if a Python object has been defined on
//               this node for the particular key (even if that object
//               is None), or false if no object has been set.
////////////////////////////////////////////////////////////////////
bool Extension<PandaNode>::
has_python_tag(const string &key) const {
  PandaNode::CDReader cdata(_this->_cycler);
  PandaNode::PythonTagData::const_iterator ti;
  ti = cdata->_python_tag_data.find(key);
  return (ti != cdata->_python_tag_data.end());
}

////////////////////////////////////////////////////////////////////
//     Function: Extension<PandaNode>::clear_python_tag
//       Access: Published
//  Description: Removes the Python object defined for this key on
//               this particular node.  After a call to
//               clear_python_tag(), has_python_tag() will return
//               false for the indicated key.
////////////////////////////////////////////////////////////////////
void Extension<PandaNode>::
clear_python_tag(const string &key) {
  Thread *current_thread = Thread::get_current_thread();
  int pipeline_stage = current_thread->get_pipeline_stage();
  nassertv(pipeline_stage == 0);

  PandaNode::CDWriter cdata(_this->_cycler, current_thread);
  PandaNode::PythonTagData::iterator ti;
  ti = cdata->_python_tag_data.find(key);
  if (ti != cdata->_python_tag_data.end()) {
    PyObject *value = (*ti).second;
    Py_XDECREF(value);
    cdata->_python_tag_data.erase(ti);
  }

  // Even though the python tag isn't recorded in the bam stream?
  _this->mark_bam_modified();
}

////////////////////////////////////////////////////////////////////
//     Function: Extension<PandaNode>::get_python_tag_keys
//       Access: Published
//  Description: Fills the given vector up with the
//               list of Python tags on this PandaNode.
//
//               It is the user's responsibility to ensure that the
//               keys vector is empty before making this call;
//               otherwise, the new files will be appended to it.
////////////////////////////////////////////////////////////////////
void Extension<PandaNode>::
get_python_tag_keys(vector_string &keys) const {
  PandaNode::CDReader cdata(_this->_cycler);
  if (!cdata->_python_tag_data.empty()) {
    PandaNode::PythonTagData::const_iterator ti = cdata->_python_tag_data.begin();
    while (ti != cdata->_python_tag_data.end()) {
      keys.push_back((*ti).first);
      ++ti;
    }
  }
}

////////////////////////////////////////////////////////////////////
//     Function: Extension<PandaNode>::get_tag_keys
//       Access: Published
//  Description: This variant on get_tag_keys returns
//               a Python list of strings.
////////////////////////////////////////////////////////////////////
PyObject *Extension<PandaNode>::
get_tag_keys() const {
  vector_string keys;
  _this->get_tag_keys(keys);

  PyObject *result = PyList_New(keys.size());
  for (size_t i = 0; i < keys.size(); ++i) {
    const string &tag_name = keys[i];
#if PY_MAJOR_VERSION >= 3
    PyObject *str = PyUnicode_FromStringAndSize(tag_name.data(), tag_name.size());
#else
    PyObject *str = PyString_FromStringAndSize(tag_name.data(), tag_name.size());
#endif
    PyList_SET_ITEM(result, i, str);
  }

  return result;
}

////////////////////////////////////////////////////////////////////
//     Function: Extension<PandaNode>::get_python_tag_keys
//       Access: Published
//  Description: This variant on get_python_tag_keys returns
//               a Python list of strings.
////////////////////////////////////////////////////////////////////
PyObject *Extension<PandaNode>::
get_python_tag_keys() const {
  vector_string keys;
  get_python_tag_keys(keys);

  PyObject *result = PyList_New(keys.size());
  for (size_t i = 0; i < keys.size(); ++i) {
    const string &tag_name = keys[i];
#if PY_MAJOR_VERSION >= 3
    PyObject *str = PyUnicode_FromStringAndSize(tag_name.data(), tag_name.size());
#else
    PyObject *str = PyString_FromStringAndSize(tag_name.data(), tag_name.size());
#endif
    PyList_SET_ITEM(result, i, str);
  }

  return result;
}

#endif  // HAVE_PYTHON

