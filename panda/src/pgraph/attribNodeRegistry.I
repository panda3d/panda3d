// Filename: attribNodeRegistry.I
// Created by:  drose (07Jul07)
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


////////////////////////////////////////////////////////////////////
//     Function: AttribNodeRegistry::get_global_ptr
//       Access: Published, Static
//  Description: 
////////////////////////////////////////////////////////////////////
INLINE AttribNodeRegistry *AttribNodeRegistry::
get_global_ptr() {
  if (_global_ptr == (AttribNodeRegistry *)NULL) {
    make_global_ptr();
  }
  return _global_ptr;
}

////////////////////////////////////////////////////////////////////
//     Function: AttribNodeRegistry::Entry::Constructor
//       Access: Public
//  Description: 
////////////////////////////////////////////////////////////////////
INLINE AttribNodeRegistry::Entry::
Entry(const NodePath &node) :
  _type(node.node()->get_type()),
  _name(node.get_name()),
  _node(node)
{
}

////////////////////////////////////////////////////////////////////
//     Function: AttribNodeRegistry::Entry::Constructor
//       Access: Public
//  Description: 
////////////////////////////////////////////////////////////////////
INLINE AttribNodeRegistry::Entry::
Entry(TypeHandle type, const string &name) :
  _type(type),
  _name(name)
{
}

////////////////////////////////////////////////////////////////////
//     Function: AttribNodeRegistry::Entry::operator <
//       Access: Public
//  Description: 
////////////////////////////////////////////////////////////////////
INLINE bool AttribNodeRegistry::Entry::
operator < (const Entry &other) const {
  if (_type != other._type) {
    return _type < other._type;
  }
  return _name < other._name;
}
