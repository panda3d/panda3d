// Filename: findApproxPath.I
// Created by:  drose (13Mar02)
//
////////////////////////////////////////////////////////////////////
//
// PANDA 3D SOFTWARE
// Copyright (c) 2001, Disney Enterprises, Inc.  All rights reserved
//
// All use of this software is subject to the terms of the Panda 3d
// Software license.  You should have received a copy of this license
// along with this source code; you will also find a current copy of
// the license at http://www.panda3d.org/license.txt .
//
// To contact the maintainers of this program write to
// panda3d@yahoogroups.com .
//
////////////////////////////////////////////////////////////////////


////////////////////////////////////////////////////////////////////
//     Function: FindApproxPath::Constructor
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
INLINE FindApproxPath::
FindApproxPath() {
  _return_hidden = true;
  _return_stashed = false;
}

////////////////////////////////////////////////////////////////////
//     Function: FindApproxPath::add_match_name
//       Access: Public
//  Description: Adds a component that must match the name of a node
//               exactly.
////////////////////////////////////////////////////////////////////
INLINE void FindApproxPath::
add_match_name(const string &name, int flags) {
  Component comp;
  comp._type = CT_match_name;
  comp._name = name;
  comp._flags = flags;
  _path.push_back(comp);
}

////////////////////////////////////////////////////////////////////
//     Function: FindApproxPath::add_match_name_glob
//       Access: Public
//  Description: Adds a component that must match the name of a node
//               using standard shell globbing rules, with wildcard
//               characters accepted.
////////////////////////////////////////////////////////////////////
INLINE void FindApproxPath::
add_match_name_glob(const string &name, int flags) {
  Component comp;
  comp._type = CT_match_name_glob;
  comp._name = name;
  comp._glob.set_pattern(name);
  comp._flags = flags;
  _path.push_back(comp);
}

////////////////////////////////////////////////////////////////////
//     Function: FindApproxPath::add_match_exact_type
//       Access: Public
//  Description: Adds a component that must match the type of a node
//               exactly, with no derived types matching.
////////////////////////////////////////////////////////////////////
INLINE void FindApproxPath::
add_match_exact_type(TypeHandle type, int flags) {
  Component comp;
  comp._type = CT_match_exact_type;
  comp._type_handle = type;
  comp._flags = flags;
  _path.push_back(comp);
}

////////////////////////////////////////////////////////////////////
//     Function: FindApproxPath::add_match_inexact_type
//       Access: Public
//  Description: Adds a component that must match the type of a node
//               or be a base class of the node's type.
////////////////////////////////////////////////////////////////////
INLINE void FindApproxPath::
add_match_inexact_type(TypeHandle type, int flags) {
  Component comp;
  comp._type = CT_match_inexact_type;
  comp._type_handle = type;
  comp._flags = flags;
  _path.push_back(comp);
}

////////////////////////////////////////////////////////////////////
//     Function: FindApproxPath::add_match_tag
//       Access: Public
//  Description: Adds a component that will match a node that has a
//               tag with the indicated key, no matter what the value
//               is.
////////////////////////////////////////////////////////////////////
INLINE void FindApproxPath::
add_match_tag(const string &name, int flags) {
  Component comp;
  comp._type = CT_match_tag;
  comp._name = name;
  comp._flags = flags;
  _path.push_back(comp);
}

////////////////////////////////////////////////////////////////////
//     Function: FindApproxPath::add_match_tag_value
//       Access: Public
//  Description: Adds a component that will match a node that has a
//               tag with the indicated key.  The value may be "*" to
//               match any value, or a particular glob pattern to
//               match only those nodes with the indicated value.
////////////////////////////////////////////////////////////////////
INLINE void FindApproxPath::
add_match_tag_value(const string &name, const string &value, int flags) {
  Component comp;
  comp._type = CT_match_tag_value;
  comp._name = name;
  comp._glob.set_pattern(value);
  comp._flags = flags;
  _path.push_back(comp);
}

////////////////////////////////////////////////////////////////////
//     Function: FindApproxPath::add_match_one
//       Access: Public
//  Description: Adds a component that will match any node (but not a
//               chain of many nodes).
////////////////////////////////////////////////////////////////////
INLINE void FindApproxPath::
add_match_one(int flags) {
  Component comp;
  comp._type = CT_match_one;
  comp._flags = flags;
  _path.push_back(comp);
}

////////////////////////////////////////////////////////////////////
//     Function: FindApproxPath::add_match_many
//       Access: Public
//  Description: Adds a component that will match a chain of zero or
//               more consecutive nodes.
////////////////////////////////////////////////////////////////////
INLINE void FindApproxPath::
add_match_many(int flags) {
  Component comp;
  comp._type = CT_match_many;
  comp._flags = flags;
  _path.push_back(comp);
}

////////////////////////////////////////////////////////////////////
//     Function: FindApproxPath::add_match_pointer
//       Access: Public
//  Description: Adds a component that must match a particular node
//               exactly, by pointer.
////////////////////////////////////////////////////////////////////
INLINE void FindApproxPath::
add_match_pointer(PandaNode *pointer, int flags) {
  Component comp;
  comp._type = CT_match_pointer;
  comp._pointer = pointer;
  comp._flags = flags;
  _path.push_back(comp);
}

////////////////////////////////////////////////////////////////////
//     Function: FindApproxPath::get_num_components
//       Access: Public
//  Description: Returns the number of components in the path.
////////////////////////////////////////////////////////////////////
INLINE int FindApproxPath::
get_num_components() const {
  return _path.size();
}

////////////////////////////////////////////////////////////////////
//     Function: FindApproxPath::is_component_match_many
//       Access: Public
//  Description: Returns true if the nth component is of type
//               match_many, which will require special handling.
////////////////////////////////////////////////////////////////////
INLINE bool FindApproxPath::
is_component_match_many(int index) const {
  nassertr(index >= 0 && index < (int)_path.size(), false);
  return (_path[index]._type == CT_match_many);
}

////////////////////////////////////////////////////////////////////
//     Function: FindApproxPath::matches_component
//       Access: Public
//  Description: Returns true if the nth component of the path matches
//               the indicated node, false otherwise.
////////////////////////////////////////////////////////////////////
INLINE bool FindApproxPath::
matches_component(int index, PandaNode *node) const {
  nassertr(index >= 0 && index < (int)_path.size(), false);
  return (_path[index].matches(node));
}

////////////////////////////////////////////////////////////////////
//     Function: FindApproxPath::matches_stashed
//       Access: Public
//  Description: Returns true if the nth component of the path matches
//               a stashed node only, false otherwise.
////////////////////////////////////////////////////////////////////
INLINE bool FindApproxPath::
matches_stashed(int index) const {
  if (index >= 0 && index < (int)_path.size()) {
    return ((_path[index]._flags & CF_stashed) != 0);
  } else {
    return false;
  }
}

////////////////////////////////////////////////////////////////////
//     Function: FindApproxPath::return_hidden
//       Access: Public
//  Description: Returns true if this path allows returning of hidden
//               nodes, false otherwise.
////////////////////////////////////////////////////////////////////
INLINE bool FindApproxPath::
return_hidden() const {
  return _return_hidden;
}

////////////////////////////////////////////////////////////////////
//     Function: FindApproxPath::return_stashed
//       Access: Public
//  Description: Returns true if this path allows returning of stashed
//               nodes, false otherwise.
////////////////////////////////////////////////////////////////////
INLINE bool FindApproxPath::
return_stashed() const {
  return _return_stashed;
}

////////////////////////////////////////////////////////////////////
//     Function: FindApproxPath::output_component
//       Access: Public
//  Description: Formats the nth component of the path to the
//               indicated output stream.
////////////////////////////////////////////////////////////////////
INLINE void FindApproxPath::
output_component(ostream &out, int index) const {
  nassertv(index >= 0 && index < (int)_path.size());
  out << _path[index];
}
