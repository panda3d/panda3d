// Filename: nodePath_ext.I
// Created by:  rdb (09Dec13)
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

#ifndef CPPPARSER
#ifdef STDFLOAT_DOUBLE
IMPORT_THIS struct Dtool_PyTypedObject Dtool_LPoint3d;
#else
IMPORT_THIS struct Dtool_PyTypedObject Dtool_LPoint3f;
#endif
#endif

////////////////////////////////////////////////////////////////////
//     Function: Extension<NodePath>::get_python_tag_keys
//       Access: Published
//  Description: Fills the given vector up with the
//               list of Python tags on this PandaNode.
//
//               It is the user's responsibility to ensure that the
//               keys vector is empty before making this call;
//               otherwise, the new files will be appended to it.
////////////////////////////////////////////////////////////////////
INLINE void Extension<NodePath>::
get_python_tag_keys(vector_string &keys) const {
  nassertv_always(!_this->is_empty());
  invoke_extension(_this->node()).get_python_tag_keys(keys);
}

////////////////////////////////////////////////////////////////////
//     Function: Filename::get_tag_keys
//       Access: Published
//  Description: This variant on get_tag_keys returns a Python list
//               of strings. Returns None if the NodePath is empty.
////////////////////////////////////////////////////////////////////
INLINE PyObject *Extension<NodePath>::
get_tag_keys() const {
  // An empty NodePath returns None
  if (_this->is_empty()) {
    Py_INCREF(Py_None);
    return Py_None;
  }
  return invoke_extension(_this->node()).get_tag_keys();
}

////////////////////////////////////////////////////////////////////
//     Function: Filename::get_python_tag_keys
//       Access: Published
//  Description: This variant on get_python_tag_keys returns a
//               Python list of strings.
//               Returns None if the NodePath is empty.
////////////////////////////////////////////////////////////////////
INLINE PyObject *Extension<NodePath>::
get_python_tag_keys() const {
  // An empty NodePath returns None
  if (_this->is_empty()) {
    Py_INCREF(Py_None);
    return Py_None;
  }
  return invoke_extension(_this->node()).get_python_tag_keys();
}

////////////////////////////////////////////////////////////////////
//     Function: Extension<NodePath>::set_python_tag
//       Access: Published
//  Description: Associates an arbitrary Python object with a
//               user-defined key which is stored on the node.  This
//               object has no meaning to Panda; but it is stored
//               indefinitely on the node until it is requested again.
//
//               Each unique key stores a different Python object.
//               There is no effective limit on the number of
//               different keys that may be stored or on the nature of
//               any one key's object.
////////////////////////////////////////////////////////////////////
INLINE void Extension<NodePath>::
set_python_tag(const string &key, PyObject *value) {
  nassertv_always(!_this->is_empty());
  invoke_extension(_this->node()).set_python_tag(key, value);
}

////////////////////////////////////////////////////////////////////
//     Function: Extension<NodePath>::get_python_tag
//       Access: Published
//  Description: Retrieves the Python object that was previously
//               set on this node for the particular key, if any.  If
//               no object has been previously set, returns None.
//               See also get_net_python_tag().
////////////////////////////////////////////////////////////////////
INLINE PyObject *Extension<NodePath>::
get_python_tag(const string &key) const {
  // An empty NodePath quietly returns no tags.  This makes
  // get_net_python_tag() easier to implement.
  if (_this->is_empty()) {
    Py_INCREF(Py_None);
    return Py_None;
  }
  return invoke_extension(_this->node()).get_python_tag(key);
}

////////////////////////////////////////////////////////////////////
//     Function: Extension<NodePath>::has_python_tag
//       Access: Published
//  Description: Returns true if a Python object has been defined on
//               this node for the particular key (even if that value
//               is the empty string), or false if no value has been
//               set.  See also has_net_python_tag().
////////////////////////////////////////////////////////////////////
INLINE bool Extension<NodePath>::
has_python_tag(const string &key) const {
  // An empty NodePath quietly has no tags.  This makes has_net_python_tag()
  // easier to implement.
  if (_this->is_empty()) {
    return false;
  }
  return invoke_extension(_this->node()).has_python_tag(key);
}

////////////////////////////////////////////////////////////////////
//     Function: Extension<NodePath>::clear_python_tag
//       Access: Published
//  Description: Removes the Python object defined for this key on this
//               particular node.  After a call to clear_python_tag(),
//               has_python_tag() will return false for the indicated
//               key.
////////////////////////////////////////////////////////////////////
INLINE void Extension<NodePath>::
clear_python_tag(const string &key) {
  nassertv_always(!_this->is_empty());
  invoke_extension(_this->node()).clear_python_tag(key);
}

////////////////////////////////////////////////////////////////////
//     Function: Extension<NodePath>::get_net_python_tag
//       Access: Published
//  Description: Returns the Python object that has been defined on
//               this node, or the nearest ancestor node, for the
//               indicated key.  If no value has been defined for the
//               indicated key on any ancestor node, returns None.
//               See also get_python_tag().
////////////////////////////////////////////////////////////////////
INLINE PyObject *Extension<NodePath>::
get_net_python_tag(const string &key) const {
  NodePath tag_np = find_net_python_tag(key);
  return invoke_extension(&tag_np).get_python_tag(key);
}

////////////////////////////////////////////////////////////////////
//     Function: Extension<NodePath>::has_net_python_tag
//       Access: Published
//  Description: Returns true if the indicated Python object has been
//               defined on this node or on any ancestor node, or
//               false otherwise.  See also has_python_tag().
////////////////////////////////////////////////////////////////////
INLINE bool Extension<NodePath>::
has_net_python_tag(const string &key) const {
  return !find_net_python_tag(key).is_empty();
}

////////////////////////////////////////////////////////////////////
//     Function: Extension<NodePath>::get_tight_bounds
//       Access: Published
//  Description: Returns the tight bounds as a 2-tuple of LPoint3
//               objects.  This is a convenience function for Python
//               users, among which the use of calc_tight_bounds
//               may be confusing.
//               Returns None if calc_tight_bounds returned false.
////////////////////////////////////////////////////////////////////
INLINE PyObject *Extension<NodePath>::
get_tight_bounds() const {
  LPoint3 *min_point = new LPoint3;
  LPoint3 *max_point = new LPoint3;

  if (_this->calc_tight_bounds(*min_point, *max_point)) {
#ifdef STDFLOAT_DOUBLE
    PyObject *min_inst = DTool_CreatePyInstance((void*) min_point, Dtool_LPoint3d, true, false);
    PyObject *max_inst = DTool_CreatePyInstance((void*) max_point, Dtool_LPoint3d, true, false);
#else
    PyObject *min_inst = DTool_CreatePyInstance((void*) min_point, Dtool_LPoint3f, true, false);
    PyObject *max_inst = DTool_CreatePyInstance((void*) max_point, Dtool_LPoint3f, true, false);
#endif
    return Py_BuildValue("NN", min_inst, max_inst);

  } else {
    Py_INCREF(Py_None);
    return Py_None;
  }
}
