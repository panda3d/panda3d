// Filename: selectiveChildNode.I
// Created by:  drose (06Mar02)
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
//       Access: Protected
//  Description:
////////////////////////////////////////////////////////////////////
INLINE SelectiveChildNode::
SelectiveChildNode(const SelectiveChildNode &copy) :
  PandaNode(copy),
  _selected_child(copy._selected_child)
{
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
