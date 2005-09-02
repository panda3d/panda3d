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

#include <math.h>


////////////////////////////////////////////////////////////////////
//     Function: AnimControl::set_play_rate
//       Access: Published
//  Description: Sets the speed of the animation, relative to its
//               "normal" speed.  Setting this number to 2.0 plays it
//               twice as fast, 0.5 half as fast.  -1.0 plays it
//               backwards, and 0.0 stops it.  The change is actually
//               retroactive to the last frame.
//
//               If you are going to change the play_rate from a
//               positive number to a negative number, or vice-versa,
//               you should do this before starting the animation.
//               The various flavors of play() and loop() do slightly
//               different behavior based on whether play_rate is
//               positive or negative.
////////////////////////////////////////////////////////////////////
INLINE void AnimControl::
set_play_rate(double play_rate) {
  _play_rate = play_rate;
}

////////////////////////////////////////////////////////////////////
//     Function: AnimControl::get_play_rate
//       Access: Published
//  Description: Returns the current speed of the animation.  See
//               set_play_rate().
////////////////////////////////////////////////////////////////////
INLINE double AnimControl::
get_play_rate() const {
  return _play_rate;
}

////////////////////////////////////////////////////////////////////
//     Function: AnimControl::get_frame_rate
//       Access: Published
//  Description: Returns the actual frame rate of the animation, based
//               on the play_rate (see set_play_rate()) and the
//               animation's base frame rate (see
//               AnimBundle::get_base_frame_rate()).  This is in
//               frames per second.
////////////////////////////////////////////////////////////////////
INLINE double AnimControl::
get_frame_rate() const {
  return get_play_rate() * get_anim()->get_base_frame_rate();
}

////////////////////////////////////////////////////////////////////
//     Function: AnimControl::get_frame
//       Access: Published
//  Description: Returns the current frame number of the animation.
////////////////////////////////////////////////////////////////////
INLINE int AnimControl::
get_frame() const {
  // We have to use floor() here instead of simply casting the number
  // to an integer, becase the frame number might have become
  // negative.
  return (int)cfloor(_frame + 0.0001);
}

////////////////////////////////////////////////////////////////////
//     Function: AnimControl::get_num_frames
//       Access: Published
//  Description: Returns the number of frames of animation.  This is
//               actually just extracted directly from the AnimBundle;
//               the function is duplicated here for convenience.  The
//               frame number will never be outside the range 0 <=
//               frame < get_num_frames().
////////////////////////////////////////////////////////////////////
INLINE int AnimControl::
get_num_frames() const {
  return get_anim()->get_num_frames();
}

////////////////////////////////////////////////////////////////////
//     Function: AnimControl::is_playing
//       Access: Published
//  Description: Returns true if the AnimControl is currently playing,
//               false otherwise.
////////////////////////////////////////////////////////////////////
INLINE bool AnimControl::
is_playing() const {
  return _playing;
}

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
