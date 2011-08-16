// Filename: switchNode.I
// Created by:  drose (31Jul02)
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
//     Function: SwitchNode::CData::Constructor
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
INLINE SwitchNode::CData::
CData() {
  _visible_child = 0;
}

////////////////////////////////////////////////////////////////////
//     Function: SwitchNode::CData::Copy Constructor
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
INLINE SwitchNode::CData::
CData(const SwitchNode::CData &copy) :
  _visible_child(copy._visible_child)
{
}

////////////////////////////////////////////////////////////////////
//     Function: SwitchNode::Constructor
//       Access: Published
//  Description:
////////////////////////////////////////////////////////////////////
INLINE SwitchNode::
SwitchNode(const string &name) :
  SelectiveChildNode(name)
{
  set_cull_callback();
}

////////////////////////////////////////////////////////////////////
//     Function: SwitchNode::set_visible_child
//       Access: Published
//  Description: Specifies the particular child of this node, by
//               index, that will be visible.
////////////////////////////////////////////////////////////////////
INLINE void SwitchNode::
set_visible_child(int index) {
  nassertv(index >= 0);
  CDWriter cdata(_cycler);
  cdata->_visible_child = index;
}
