// Filename: animInterface.I
// Created by:  drose (20Sep05)
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
//     Function: AnimInterface::play
//       Access: Published
//  Description: Runs the entire animation from beginning to end and
//               stops.
////////////////////////////////////////////////////////////////////
INLINE void AnimInterface::
play() {
  play(0, get_num_frames() - 1);
}

////////////////////////////////////////////////////////////////////
//     Function: AnimInterface::loop
//       Access: Published
//  Description: Starts the entire animation looping.  If restart is
//               true, the animation is restarted from the beginning;
//               otherwise, it continues from the current frame.
////////////////////////////////////////////////////////////////////
INLINE void AnimInterface::
loop(bool restart) {
  loop(restart, 0, get_num_frames() - 1);
}

////////////////////////////////////////////////////////////////////
//     Function: AnimInterface::pingpong
//       Access: Published
//  Description: Starts the entire animation bouncing back and forth
//               between its first frame and last frame.  If restart
//               is true, the animation is restarted from the
//               beginning; otherwise, it continues from the current
//               frame.
////////////////////////////////////////////////////////////////////
INLINE void AnimInterface::
pingpong(bool restart) {
  pingpong(restart, 0, get_num_frames() - 1);
}

////////////////////////////////////////////////////////////////////
//     Function: AnimInterface::stop
//       Access: Published
//  Description: Stops a currently playing or looping animation right
//               where it is.  The animation remains posed at the
//               current frame.
////////////////////////////////////////////////////////////////////
INLINE void AnimInterface::
stop() {
  pose(get_frame());
}

////////////////////////////////////////////////////////////////////
//     Function: AnimInterface::set_play_rate
//       Access: Published
//  Description: Changes the rate at which the animation plays.  1.0
//               is the normal speed, 2.0 is twice normal speed, and
//               0.5 is half normal speed.  0.0 is legal to pause the
//               animation, and a negative value will play the
//               animation backwards.
////////////////////////////////////////////////////////////////////
INLINE void AnimInterface::
set_play_rate(double play_rate) {
  internal_set_rate(_frame_rate, play_rate);
}

////////////////////////////////////////////////////////////////////
//     Function: AnimInterface::get_play_rate
//       Access: Published
//  Description: Returns the rate at which the animation plays.  See
//               set_play_rate().
////////////////////////////////////////////////////////////////////
INLINE double AnimInterface::
get_play_rate() const {
  return _play_rate;
}

////////////////////////////////////////////////////////////////////
//     Function: AnimInterface::get_frame_rate
//       Access: Published
//  Description: Returns the native frame rate of the animation.  This
//               is the number of frames per second that will elapse
//               when the play_rate is set to 1.0.  It is a fixed
//               property of the animation and may not be adjusted by
//               the user.
////////////////////////////////////////////////////////////////////
INLINE double AnimInterface::
get_frame_rate() const {
  return _frame_rate;
}

////////////////////////////////////////////////////////////////////
//     Function: AnimInterface::get_num_frames
//       Access: Published
//  Description: Returns the number of frames in the animation.  This
//               is a fixed property of the animation and may not be
//               adjusted by the user.
////////////////////////////////////////////////////////////////////
INLINE int AnimInterface::
get_num_frames() const {
  return _num_frames;
}

////////////////////////////////////////////////////////////////////
//     Function: AnimInterface::get_frame
//       Access: Published
//  Description: Returns the current integer frame number.
//               This number will be in the range 0 <= f <
//               get_num_frames().
////////////////////////////////////////////////////////////////////
INLINE int AnimInterface::
get_frame() const {
  return cmod(get_full_frame(), get_num_frames());
}

////////////////////////////////////////////////////////////////////
//     Function: AnimInterface::get_frac
//       Access: Published
//  Description: Returns the fractional part of the current frame.
//               Normally, this is in the range 0.0 <= f < 1.0, but in
//               the one special case of an animation playing to its
//               end frame and stopping, it might exactly equal 1.0.
//
//               It will always be true that get_full_frame() +
//               get_frac() == get_full_fframe().
////////////////////////////////////////////////////////////////////
INLINE double AnimInterface::
get_frac() const {
  return get_full_fframe() - (double)get_full_frame();
}

////////////////////////////////////////////////////////////////////
//     Function: AnimInterface::set_frame_rate
//       Access: Protected
//  Description: Should be called by a derived class to specify the
//               native frame rate of the animation.  It is legal to
//               call this after the animation has already started.
////////////////////////////////////////////////////////////////////
INLINE void AnimInterface::
set_frame_rate(double frame_rate) {
  internal_set_rate(frame_rate, _play_rate);
}

////////////////////////////////////////////////////////////////////
//     Function: AnimInterface::set_num_frames
//       Access: Protected
//  Description: Should be called by a derived class to specify the
//               number of frames of the animation.  It is legal to
//               call this after the animation has already started,
//               but doing so may suddenly change the apparent current
//               frame number.
////////////////////////////////////////////////////////////////////
INLINE void AnimInterface::
set_num_frames(int num_frames) {
  _num_frames = num_frames;
}

INLINE ostream &
operator << (ostream &out, const AnimInterface &ai) {
  ai.output(out);
  return out;
}
