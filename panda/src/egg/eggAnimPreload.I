// Filename: eggAnimPreload.I
// Created by:  drose (06Aug08)
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
//     Function: EggAnimPreload::Constructor
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
INLINE EggAnimPreload::
EggAnimPreload(const string &name) : EggNode(name) {
  _has_fps = false;
  _has_num_frames = false;
}


////////////////////////////////////////////////////////////////////
//     Function: EggAnimPreload::Copy constructor
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
INLINE EggAnimPreload::
EggAnimPreload(const EggAnimPreload &copy) : 
  EggNode(copy),
  _fps(copy._fps),
  _has_fps(copy._has_fps),
  _num_frames(copy._num_frames),
  _has_num_frames(copy._has_num_frames)
{
}


////////////////////////////////////////////////////////////////////
//     Function: EggAnimPreload::Copy assignment operator
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
INLINE EggAnimPreload &EggAnimPreload::
operator = (const EggAnimPreload &copy) {
  EggNode::operator = (copy);
  _fps = copy._fps;
  _has_fps = copy._has_fps;
  _num_frames = copy._num_frames;
  _has_num_frames = copy._has_num_frames;

  return *this;
}


////////////////////////////////////////////////////////////////////
//     Function: EggAnimPreload::set_fps
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
INLINE void EggAnimPreload::
set_fps(double fps) {
  _fps = fps;
  _has_fps = true;
}


////////////////////////////////////////////////////////////////////
//     Function: EggAnimPreload::clear_fps
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
INLINE void EggAnimPreload::
clear_fps() {
  _has_fps = false;
}

////////////////////////////////////////////////////////////////////
//     Function: EggAnimPreload::has_fps
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
INLINE bool EggAnimPreload::
has_fps() const {
  return _has_fps;
}

////////////////////////////////////////////////////////////////////
//     Function: EggAnimPreload::get_fps
//       Access: Public
//  Description: This is only valid if has_fps() returns true.
////////////////////////////////////////////////////////////////////
INLINE double EggAnimPreload::
get_fps() const {
  nassertr(has_fps(), 0.0);
  return _fps;
}

////////////////////////////////////////////////////////////////////
//     Function: EggAnimPreload::set_num_frames
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
INLINE void EggAnimPreload::
set_num_frames(int num_frames) {
  _num_frames = num_frames;
  _has_num_frames = true;
}


////////////////////////////////////////////////////////////////////
//     Function: EggAnimPreload::clear_num_frames
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
INLINE void EggAnimPreload::
clear_num_frames() {
  _has_num_frames = false;
}

////////////////////////////////////////////////////////////////////
//     Function: EggAnimPreload::has_num_frames
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
INLINE bool EggAnimPreload::
has_num_frames() const {
  return _has_num_frames;
}

////////////////////////////////////////////////////////////////////
//     Function: EggAnimPreload::get_num_frames
//       Access: Public
//  Description: This is only valid if has_num_frames() returns true.
////////////////////////////////////////////////////////////////////
INLINE int EggAnimPreload::
get_num_frames() const {
  nassertr(has_num_frames(), 0);
  return _num_frames;
}
