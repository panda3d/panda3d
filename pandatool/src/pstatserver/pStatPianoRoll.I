// Filename: pStatPianoRoll.I
// Created by:  drose (18Jul00)
// 
////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////////
//     Function: PStatPianoRoll::set_horizontal_scale
//       Access: Public
//  Description: Changes the amount of time the width of the
//               horizontal axis represents.  This may force a redraw.
////////////////////////////////////////////////////////////////////
INLINE void PStatPianoRoll::
set_horizontal_scale(double time_width) {
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
INLINE double PStatPianoRoll::
get_horizontal_scale() const {
  return _time_width;
}

////////////////////////////////////////////////////////////////////
//     Function: PStatPianoRoll::timestamp_to_pixel
//       Access: Public
//  Description: Converts a timestamp to a horizontal pixel offset.
////////////////////////////////////////////////////////////////////
INLINE int PStatPianoRoll::
timestamp_to_pixel(double time) const {
  return (int)((double)_xsize * (time - _start_time) / _time_width);
}

////////////////////////////////////////////////////////////////////
//     Function: PStatPianoRoll::pixel_to_timestamp
//       Access: Public
//  Description: Converts a horizontal pixel offset to a timestamp.
////////////////////////////////////////////////////////////////////
INLINE double PStatPianoRoll::
pixel_to_timestamp(int x) const {
  return _time_width * (double)x / (double)_xsize + _start_time;
}
