// Filename: animControl.I
// Created by:  drose (19Feb99)
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
