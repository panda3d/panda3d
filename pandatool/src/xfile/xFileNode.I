// Filename: xFileNode.I
// Created by:  drose (03Oct04)
//
////////////////////////////////////////////////////////////////////
//
// PANDA 3D SOFTWARE
// Copyright (c) 2001 - 2004, Disney Enterprises, Inc.  All rights reserved
//
// All use of this software is subject to the terms of the Panda 3d
// Software license.  You should have received a copy of this license
// along with this source code; you will also find a current copy of
// the license at http://etc.cmu.edu/panda3d/docs/license/ .
//
// To contact the maintainers of this program write to
// panda3d-general@lists.sourceforge.net .
//
////////////////////////////////////////////////////////////////////


////////////////////////////////////////////////////////////////////
//     Function: XFileNode::get_num_children
//       Access: Public
//  Description: 
////////////////////////////////////////////////////////////////////
INLINE XFile *XFileNode::
get_x_file() const {
  return _x_file;
}

////////////////////////////////////////////////////////////////////
//     Function: XFileNode::get_num_children
//       Access: Public
//  Description: Returns the list of children of this node.  This
//               list includes templates as well as data objects.
////////////////////////////////////////////////////////////////////
INLINE int XFileNode::
get_num_children() const {
  return _children.size();
}

////////////////////////////////////////////////////////////////////
//     Function: XFileNode::get_child
//       Access: Public
//  Description: Returns the nth child of this node.  This list
//               includes templates as well as data objects.
////////////////////////////////////////////////////////////////////
INLINE XFileNode *XFileNode::
get_child(int n) const {
  nassertr(n >= 0 && n < (int)_children.size(), NULL);
  return _children[n];
}

////////////////////////////////////////////////////////////////////
//     Function: XFileNode::get_num_objects
//       Access: Public
//  Description: Returns the list of child objects of this node.  This
//               list does not include template definitions; it is
//               strictly the list of children that are also data
//               objects (instances of templates).
////////////////////////////////////////////////////////////////////
INLINE int XFileNode::
get_num_objects() const {
  return _objects.size();
}

////////////////////////////////////////////////////////////////////
//     Function: XFileNode::get_object
//       Access: Public
//  Description: Returns the nth child object of this node.  This
//               list does not include template definitions; it is
//               strictly the list of children that are also data
//               objects (instances of templates).
////////////////////////////////////////////////////////////////////
INLINE XFileDataNode *XFileNode::
get_object(int n) const {
  nassertr(n >= 0 && n < (int)_objects.size(), NULL);
  return _objects[n];
}
