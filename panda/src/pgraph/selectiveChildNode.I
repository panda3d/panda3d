// Filename: selectiveChildNode.I
// Created by:  drose (06Mar02)
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
//     Function: SelectiveChildNode::Constructor
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
INLINE SelectiveChildNode::
SelectiveChildNode(const string &name) :
  PandaNode(name),
  _selected_child(0)
{
}

////////////////////////////////////////////////////////////////////
//     Function: SelectiveChildNode::Copy Constructor
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
INLINE SelectiveChildNode::
SelectiveChildNode(const SelectiveChildNode &copy) :
  PandaNode(copy),
  _selected_child(copy._selected_child)
{
}

////////////////////////////////////////////////////////////////////
//     Function: SelectiveChildNode::Copy Assignment Operator
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
INLINE void SelectiveChildNode::
operator = (const SelectiveChildNode &copy) {
  PandaNode::operator = (copy);
  _selected_child = copy._selected_child;
}

////////////////////////////////////////////////////////////////////
//     Function: SelectiveChildNode::select_child
//       Access: Protected
//  Description: Selects the indicated child for rendering.  This is
//               normally called during the cull_callback() method,
//               but it may be called at any time.
////////////////////////////////////////////////////////////////////
INLINE void SelectiveChildNode::
select_child(int n) {
  _selected_child = n;
}
