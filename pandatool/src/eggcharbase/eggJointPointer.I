// Filename: eggJointPointer.I
// Created by:  drose (20Jul03)
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
//     Function: EggJointPointer::get_num_rebuild_frames
//       Access: Public
//  Description: Returns the number of rebuild frames that have been
//               added so far.
////////////////////////////////////////////////////////////////////
INLINE int EggJointPointer::
get_num_rebuild_frames() const {
  return _rebuild_frames.size();
}

////////////////////////////////////////////////////////////////////
//     Function: EggJointPointer::get_rebuild_frame
//       Access: Public
//  Description: Returns the nth matrix that has been added to the set
//               of rebuild frames.
////////////////////////////////////////////////////////////////////
INLINE const LMatrix4d &EggJointPointer::
get_rebuild_frame(int n) const {
  nassertr(n >= 0 && n < (int)_rebuild_frames.size(), LMatrix4d::ident_mat());
  return _rebuild_frames[n];
}
