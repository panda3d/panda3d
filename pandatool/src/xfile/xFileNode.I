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
//  Description: 
////////////////////////////////////////////////////////////////////
INLINE int XFileNode::
get_num_children() const {
  return _children.size();
}

////////////////////////////////////////////////////////////////////
//     Function: XFileNode::get_child
//       Access: Public
//  Description: 
////////////////////////////////////////////////////////////////////
INLINE XFileNode *XFileNode::
get_child(int n) const {
  nassertr(n >= 0 && n < (int)_children.size(), NULL);
  return _children[n];
}
