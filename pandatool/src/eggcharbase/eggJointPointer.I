// Filename: eggJointPointer.I
// Created by:  drose (20Jul03)
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

////////////////////////////////////////////////////////////////////
//     Function: EggJointPointer::clear_net_frames
//       Access: Public
//  Description: Resets the cache of net frames for this joint.
////////////////////////////////////////////////////////////////////
INLINE void EggJointPointer::
clear_net_frames() {
  _net_frames.clear();
}

////////////////////////////////////////////////////////////////////
//     Function: EggJointPointer::add_net_frame
//       Access: Public, Virtual
//  Description: Adds a new frame to the set of net frames.  This is
//               used to cache the net transform from the root for
//               this particular joint.
////////////////////////////////////////////////////////////////////
INLINE void EggJointPointer::
add_net_frame(const LMatrix4d &mat) {
  _net_frames.push_back(mat);
}

////////////////////////////////////////////////////////////////////
//     Function: EggJointPointer::get_num_net_frames
//       Access: Public
//  Description: Returns the number of net frames that have been
//               added so far.
////////////////////////////////////////////////////////////////////
INLINE int EggJointPointer::
get_num_net_frames() const {
  return _net_frames.size();
}

////////////////////////////////////////////////////////////////////
//     Function: EggJointPointer::get_net_frame
//       Access: Public
//  Description: Returns the nth matrix that has been added to the set
//               of net frames.
////////////////////////////////////////////////////////////////////
INLINE const LMatrix4d &EggJointPointer::
get_net_frame(int n) const {
  nassertr(n >= 0 && n < (int)_net_frames.size(), LMatrix4d::ident_mat());
  return _net_frames[n];
}

////////////////////////////////////////////////////////////////////
//     Function: EggJointPointer::clear_net_frame_invs
//       Access: Public
//  Description: Resets the cache of net_inv frames for this joint.
////////////////////////////////////////////////////////////////////
INLINE void EggJointPointer::
clear_net_frame_invs() {
  _net_frame_invs.clear();
}

////////////////////////////////////////////////////////////////////
//     Function: EggJointPointer::add_net_frame_inv
//       Access: Public, Virtual
//  Description: Adds a new frame to the set of net_inv frames.  This is
//               used to cache the inverse net transform from the root
//               for this particular joint.
////////////////////////////////////////////////////////////////////
INLINE void EggJointPointer::
add_net_frame_inv(const LMatrix4d &mat) {
  _net_frame_invs.push_back(mat);
}

////////////////////////////////////////////////////////////////////
//     Function: EggJointPointer::get_num_net_frame_invs
//       Access: Public
//  Description: Returns the number of net_inv frames that have been
//               added so far.
////////////////////////////////////////////////////////////////////
INLINE int EggJointPointer::
get_num_net_frame_invs() const {
  return _net_frame_invs.size();
}

////////////////////////////////////////////////////////////////////
//     Function: EggJointPointer::get_net_frame_inv
//       Access: Public
//  Description: Returns the nth matrix that has been added to the set
//               of net_inv frames.
////////////////////////////////////////////////////////////////////
INLINE const LMatrix4d &EggJointPointer::
get_net_frame_inv(int n) const {
  nassertr(n >= 0 && n < (int)_net_frame_invs.size(), LMatrix4d::ident_mat());
  return _net_frame_invs[n];
}
