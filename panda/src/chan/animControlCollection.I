// Filename: animControlCollection.I
// Created by:  drose (22Feb00)
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
//     Function: AnimControlCollection::play
//       Access: Public
//  Description: Starts the named animation playing.
////////////////////////////////////////////////////////////////////
INLINE bool AnimControlCollection::
play(const string &anim_name) {
  AnimControl *control = find_anim(anim_name);
  if (control == (AnimControl *)NULL) {
    return false;
  }
  _last_started_control = control;
  control->play();
  return true;
}

////////////////////////////////////////////////////////////////////
//     Function: AnimControlCollection::play
//       Access: Public
//  Description: Starts the named animation playing.
////////////////////////////////////////////////////////////////////
INLINE bool AnimControlCollection::
play(const string &anim_name, int from, int to) {
  AnimControl *control = find_anim(anim_name);
  if (control == (AnimControl *)NULL) {
    return false;
  }
  _last_started_control = control;
  control->play(from, to);
  return true;
}

////////////////////////////////////////////////////////////////////
//     Function: AnimControlCollection::loop
//       Access: Public
//  Description: Starts the named animation looping.
////////////////////////////////////////////////////////////////////
INLINE bool AnimControlCollection::
loop(const string &anim_name, bool restart) {
  AnimControl *control = find_anim(anim_name);
  if (control == (AnimControl *)NULL) {
    return false;
  }
  _last_started_control = control;
  control->loop(restart);
  return true;
}

////////////////////////////////////////////////////////////////////
//     Function: AnimControlCollection::loop
//       Access: Public
//  Description: Starts the named animation looping.
////////////////////////////////////////////////////////////////////
INLINE bool AnimControlCollection::
loop(const string &anim_name, bool restart, int from, int to) {
  AnimControl *control = find_anim(anim_name);
  if (control == (AnimControl *)NULL) {
    return false;
  }
  _last_started_control = control;
  control->loop(restart, from, to);
  return true;
}

////////////////////////////////////////////////////////////////////
//     Function: AnimControlCollection::stop
//       Access: Public
//  Description: Stops the named animation.
////////////////////////////////////////////////////////////////////
INLINE bool AnimControlCollection::
stop(const string &anim_name) {
  AnimControl *control = find_anim(anim_name);
  if (control == (AnimControl *)NULL) {
    return false;
  }
  control->stop();
  return true;
}


////////////////////////////////////////////////////////////////////
//     Function: AnimControlCollection::pose
//       Access: Public
//  Description: Sets to a particular frame in the named animation.
////////////////////////////////////////////////////////////////////
INLINE bool AnimControlCollection::
pose(const string &anim_name, int frame) {
  AnimControl *control = find_anim(anim_name);
  if (control == (AnimControl *)NULL) {
    return false;
  }
  _last_started_control = control;
  control->pose(frame);
  return true;
}

////////////////////////////////////////////////////////////////////
//     Function: AnimControlCollection::get_frame
//       Access: Public
//  Description: Returns the current frame in the named animation, or
//               0 if the animation is not found.
////////////////////////////////////////////////////////////////////
INLINE int AnimControlCollection::
get_frame(const string &anim_name) const {
  AnimControl *control = find_anim(anim_name);
  if (control == (AnimControl *)NULL) {
    return 0;
  }
  return control->get_frame();
}

////////////////////////////////////////////////////////////////////
//     Function: AnimControlCollection::get_frame
//       Access: Public
//  Description: Returns the current frame in the last-started
//               animation.
////////////////////////////////////////////////////////////////////
INLINE int AnimControlCollection::
get_frame() const {
  if (_last_started_control == (AnimControl *)NULL) {
    return 0;
  }
  return _last_started_control->get_frame();
}

////////////////////////////////////////////////////////////////////
//     Function: AnimControlCollection::is_playing
//       Access: Public
//  Description: Returns true if the named animation is currently
//               playing, false otherwise.
////////////////////////////////////////////////////////////////////
INLINE bool AnimControlCollection::
is_playing(const string &anim_name) const {
  AnimControl *control = find_anim(anim_name);
  if (control == (AnimControl *)NULL) {
    return false;
  }
  return control->is_playing();
}

////////////////////////////////////////////////////////////////////
//     Function: AnimControlCollection::is_playing
//       Access: Public
//  Description: Returns true if the last-started animation is
//               currently playing, false otherwise.
////////////////////////////////////////////////////////////////////
INLINE bool AnimControlCollection::
is_playing() const {
  if (_last_started_control == (AnimControl *)NULL) {
    return false;
  }
  return _last_started_control->is_playing();
}

////////////////////////////////////////////////////////////////////
//     Function: AnimControlCollection::get_num_frames
//       Access: Public
//  Description: Returns the total number of frames in the named
//               animation, or 0 if the animation is not found.
////////////////////////////////////////////////////////////////////
INLINE int AnimControlCollection::
get_num_frames(const string &anim_name) const {
  AnimControl *control = find_anim(anim_name);
  if (control == (AnimControl *)NULL) {
    return 0;
  }
  return control->get_num_frames();
}

////////////////////////////////////////////////////////////////////
//     Function: AnimControlCollection::get_num_frames
//       Access: Public
//  Description: Returns the total number of frames in the
//               last-started animation.
////////////////////////////////////////////////////////////////////
INLINE int AnimControlCollection::
get_num_frames() const {
  if (_last_started_control == (AnimControl *)NULL) {
    return 0;
  }
  return _last_started_control->get_num_frames();
}

INLINE ostream &
operator << (ostream &out, const AnimControlCollection &collection) {
  collection.output(out);
  return out;
}

