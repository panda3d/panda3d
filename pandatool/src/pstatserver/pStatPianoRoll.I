// Filename: pStatPianoRoll.I
// Created by:  drose (18Jul00)
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
//     Function: PStatPianoRoll::set_horizontal_scale
//       Access: Public
//  Description: Changes the amount of time the width of the
//               horizontal axis represents.  This may force a redraw.
////////////////////////////////////////////////////////////////////
INLINE void PStatPianoRoll::
set_horizontal_scale(float time_width) {
  if (_time_width != time_width) {
    _time_width = time_width;
    normal_guide_bars();
    force_redraw();
  }
}

////////////////////////////////////////////////////////////////////
//     Function: PStatPianoRoll::get_horizontal_scale
//       Access: Public
//  Description: Returns the amount of total time the width of the
//               horizontal axis represents.
////////////////////////////////////////////////////////////////////
INLINE float PStatPianoRoll::
get_horizontal_scale() const {
  return _time_width;
}

////////////////////////////////////////////////////////////////////
//     Function: PStatPianoRoll::timestamp_to_pixel
//       Access: Public
//  Description: Converts a timestamp to a horizontal pixel offset.
////////////////////////////////////////////////////////////////////
INLINE int PStatPianoRoll::
timestamp_to_pixel(float time) const {
  return (int)((float)_xsize * (time - _start_time) / _time_width);
}

////////////////////////////////////////////////////////////////////
//     Function: PStatPianoRoll::pixel_to_timestamp
//       Access: Public
//  Description: Converts a horizontal pixel offset to a timestamp.
////////////////////////////////////////////////////////////////////
INLINE float PStatPianoRoll::
pixel_to_timestamp(int x) const {
  return _time_width * (float)x / (float)_xsize + _start_time;
}

////////////////////////////////////////////////////////////////////
//     Function: PStatPianoRoll::height_to_pixel
//       Access: Public
//  Description: Converts a value (i.e. a "height" in the strip chart)
//               to a horizontal pixel offset.
////////////////////////////////////////////////////////////////////
INLINE int PStatPianoRoll::
height_to_pixel(float value) const {
  return (int)((float)_xsize * value / _time_width);
}

////////////////////////////////////////////////////////////////////
//     Function: PStatPianoRoll::pixel_to_height
//       Access: Public
//  Description: Converts a horizontal pixel offset to a value (a
//               "height" in the strip chart).
////////////////////////////////////////////////////////////////////
INLINE float PStatPianoRoll::
pixel_to_height(int x) const {
  return _time_width * (float)x / (float)_xsize;
}
