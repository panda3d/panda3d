// Filename: eggJointData.I
// Created by:  drose (23Feb01)
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
//     Function: EggJointData::get_parent
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
INLINE EggJointData *EggJointData::
get_parent() const {
  return _parent;
}

////////////////////////////////////////////////////////////////////
//     Function: EggJointData::get_num_children
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
INLINE int EggJointData::
get_num_children() const {
  return _children.size();
}

////////////////////////////////////////////////////////////////////
//     Function: EggJointData::get_child
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
INLINE EggJointData *EggJointData::
get_child(int n) const {
  nassertr(n >= 0 && n < (int)_children.size(), (EggJointData *)NULL);
  return _children[n];
}
