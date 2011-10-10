// Filename: animPreloadTable.I
// Created by:  drose (05Aug08)
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
//     Function: AnimPreloadTable::AnimRecord::Constructor
//       Access: Public
//  Description: 
////////////////////////////////////////////////////////////////////
INLINE AnimPreloadTable::AnimRecord::
AnimRecord() {
}

////////////////////////////////////////////////////////////////////
//     Function: AnimPreloadTable::AnimRecord::operator <
//       Access: Public
//  Description: 
////////////////////////////////////////////////////////////////////
INLINE bool AnimPreloadTable::AnimRecord::
operator < (const AnimRecord &other) const {
  return _basename < other._basename;
}

////////////////////////////////////////////////////////////////////
//     Function: AnimPreloadTable::get_basename
//       Access: Published
//  Description: Returns the basename stored for the nth animation
//               record.  See find_anim().
////////////////////////////////////////////////////////////////////
INLINE string AnimPreloadTable::
get_basename(int n) const {
  nassertr(n >= 0 && n < (int)_anims.size(), string());
  consider_sort();
  return _anims[n]._basename;
}

////////////////////////////////////////////////////////////////////
//     Function: AnimPreloadTable::get_base_frame_rate
//       Access: Published
//  Description: Returns the frame rate stored for the nth animation
//               record.
////////////////////////////////////////////////////////////////////
INLINE PN_stdfloat AnimPreloadTable::
get_base_frame_rate(int n) const {
  nassertr(n >= 0 && n < (int)_anims.size(), 0.0f);
  consider_sort();
  return _anims[n]._base_frame_rate;
}

////////////////////////////////////////////////////////////////////
//     Function: AnimPreloadTable::get_num_frames
//       Access: Published
//  Description: Returns the number of frames stored for the nth
//               animation record.
////////////////////////////////////////////////////////////////////
INLINE int AnimPreloadTable::
get_num_frames(int n) const {
  nassertr(n >= 0 && n < (int)_anims.size(), 0);
  consider_sort();
  return _anims[n]._num_frames;
}

////////////////////////////////////////////////////////////////////
//     Function: AnimPreloadTable::consider_sort
//       Access: Private
//  Description: Ensures the table is kept in alphabetical order by
//               basename.
////////////////////////////////////////////////////////////////////
INLINE void AnimPreloadTable::
consider_sort() const {
  if (_needs_sort) {
    ((AnimPreloadTable *)this)->_anims.sort();
    ((AnimPreloadTable *)this)->_needs_sort = false;
  }
}
