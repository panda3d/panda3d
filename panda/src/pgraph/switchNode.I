// Filename: switchNode.I
// Created by:  drose (31Jul02)
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
}

////////////////////////////////////////////////////////////////////
//     Function: SwitchNode::set_visible_child
//       Access: Published
//  Description: Specifies the particular child of this node, by
//               index, that will be visible.
////////////////////////////////////////////////////////////////////
INLINE void SwitchNode::
set_visible_child(int index) {
  CDWriter cdata(_cycler);
  cdata->_visible_child = index;
}
