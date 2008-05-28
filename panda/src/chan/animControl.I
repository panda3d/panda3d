// Filename: animControl.I
// Created by:  drose (19Feb99)
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
//     Function: AnimControl::get_anim
//       Access: Published
//  Description: Returns the AnimBundle bound in with this
//               AnimControl.
////////////////////////////////////////////////////////////////////
INLINE AnimBundle *AnimControl::
get_anim() const {
  return _anim;
}

////////////////////////////////////////////////////////////////////
//     Function: AnimControl::get_channel_index
//       Access: Published
//  Description: Returns the particular channel index associated with
//               this AnimControl.  This channel index is the slot on
//               which each AnimGroup is bound to its associated
//               PartGroup, for each joint in the animation.
//
//               It will be true that
//               get_part()->find_child("n")->get_bound(get_channel_index())
//               == get_anim()->find_child("n"), for each joint "n".
////////////////////////////////////////////////////////////////////
INLINE int AnimControl::
get_channel_index() const {
  return _channel_index;
}

////////////////////////////////////////////////////////////////////
//     Function: AnimControl::get_bound_joints
//       Access: Published
//  Description: Returns the subset of joints controlled by this
//               AnimControl.  Most of the time, this will be
//               BitArray::all_on(), for a normal full-body animation.
//               For a subset animation, however, this will be just a
//               subset of those bits, corresponding to the set of
//               joints and sliders actually bound (as enumerated by
//               bind_hierarchy() in depth-first LIFO order).
////////////////////////////////////////////////////////////////////
INLINE const BitArray &AnimControl::
get_bound_joints() const {
  return _bound_joints;
}
