// Filename: pgWaitBar.I
// Created by:  drose (14Mar02)
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
//     Function: PGWaitBar::set_range
//       Access: Published
//  Description: Sets the value at which the WaitBar indicates 100%.
////////////////////////////////////////////////////////////////////
INLINE void PGWaitBar:: 
set_range(float range) {
  _range = range;
  _bar_state = -1;
}

////////////////////////////////////////////////////////////////////
//     Function: PGWaitBar::get_range
//       Access: Published
//  Description: Returns the value at which the WaitBar indicates 100%.
////////////////////////////////////////////////////////////////////
INLINE float PGWaitBar:: 
get_range() const {
  return _range;
}

////////////////////////////////////////////////////////////////////
//     Function: PGWaitBar::set_value
//       Access: Published
//  Description: Sets the current value of the bar.  This should range
//               between 0 and get_range().
////////////////////////////////////////////////////////////////////
INLINE void PGWaitBar:: 
set_value(float value) {
  _value = value;
  _bar_state = -1;
}

////////////////////////////////////////////////////////////////////
//     Function: PGWaitBar::get_value
//       Access: Published
//  Description: Returns the current value of the bar.
////////////////////////////////////////////////////////////////////
INLINE float PGWaitBar:: 
get_value() const {
  return _value;
}

////////////////////////////////////////////////////////////////////
//     Function: PGWaitBar::get_percent
//       Access: Published
//  Description: Returns the percentage complete.
////////////////////////////////////////////////////////////////////
INLINE float PGWaitBar:: 
get_percent() const {
  return (_value / _range) * 100.0f;
}

////////////////////////////////////////////////////////////////////
//     Function: PGWaitBar::set_bar_style
//       Access: Published
//  Description: Sets the kind of frame that is drawn on top of the
//               WaitBar to represent the amount completed.
////////////////////////////////////////////////////////////////////
INLINE void PGWaitBar:: 
set_bar_style(const PGFrameStyle &style) {
  _bar_style = style;
  _bar_state = -1;
}

////////////////////////////////////////////////////////////////////
//     Function: PGWaitBar::get_bar_style
//       Access: Published
//  Description: Returns the kind of frame that is drawn on top of the
//               WaitBar to represent the amount completed.
////////////////////////////////////////////////////////////////////
INLINE PGFrameStyle PGWaitBar:: 
get_bar_style() const {
  return _bar_style;
}
