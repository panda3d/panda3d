// Filename: switchNodeOne.I
// Created by:  drose (15May01)
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
//     Function: SwitchNodeOne::Constructor
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
INLINE SwitchNodeOne::
SwitchNodeOne(const string &name) :
  SwitchNode(name)
{
  _selected_child_index = 0;
}

////////////////////////////////////////////////////////////////////
//     Function: SwitchNodeOne::Copy Constructor
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
INLINE SwitchNodeOne::
SwitchNodeOne(const SwitchNodeOne &copy) :
  SwitchNode(copy),
  _selected_child_index(copy._selected_child_index)
{
}

////////////////////////////////////////////////////////////////////
//     Function: SwitchNodeOne::Copy Assignment Operator
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
INLINE void SwitchNodeOne::
operator = (const SwitchNodeOne &copy) {
  SwitchNode::operator = (copy);
  _selected_child_index = copy._selected_child_index;
}

////////////////////////////////////////////////////////////////////
//     Function: SwitchNodeOne::select_child
//       Access: Public
//  Description: Indicates the particular child which should be
//               visible.
////////////////////////////////////////////////////////////////////
INLINE void SwitchNodeOne::
select_child(int index) {
  if (switchnode_cat.is_debug()) {
    if (_selected_child_index != index) {
      switchnode_cat.debug()
        << *this << " selecting child " << index << "\n";
    }
  }

  _selected_child_index = index;
}
