// Filename: switchNodeOne.cxx
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

#include "switchNodeOne.h"

TypeHandle SwitchNodeOne::_type_handle;


////////////////////////////////////////////////////////////////////
//     Function: SwitchNodeOne::is_child_visible
//       Access: Public, Virtual
//  Description: Returns true if the indicated child is one that
//               should be considered visible (rendered), false if it
//               should be suppressed.
////////////////////////////////////////////////////////////////////
bool SwitchNodeOne::
is_child_visible(TypeHandle, int index) {
  return (index == _selected_child_index);
}
