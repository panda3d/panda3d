// Filename: qppgWaitBar.I
// Created by:  drose (14Mar02)
//
////////////////////////////////////////////////////////////////////
//
// PANDA 3D SOFTWARE
// Copyright (c) 2001, Disney Enterprises, Inc.  All rights reserved
//
// All use of this software is subject to the terms of the Panda 3d
// Software license.  You should have received a copy of this license
// along with this source code; you will also find a current copy of
// the license at http://www.panda3d.org/license.txt .
//
// To contact the maintainers of this program write to
// panda3d@yahoogroups.com .
//
////////////////////////////////////////////////////////////////////


////////////////////////////////////////////////////////////////////
//     Function: qpPGWaitBar::set_range
//       Access: Published
//  Description: Sets the value at which the WaitBar indicates 100%.
////////////////////////////////////////////////////////////////////
INLINE void qpPGWaitBar:: 
set_range(float range) {
  _range = range;
  _bar_state = -1;
}

////////////////////////////////////////////////////////////////////
//     Function: qpPGWaitBar::get_range
//       Access: Published
//  Description: Returns the value at which the WaitBar indicates 100%.
////////////////////////////////////////////////////////////////////
INLINE float qpPGWaitBar:: 
get_range() const {
  return _range;
}

////////////////////////////////////////////////////////////////////
//     Function: qpPGWaitBar::set_value
//       Access: Published
//  Description: Sets the current value of the bar.  This should range
//               between 0 and get_range().
////////////////////////////////////////////////////////////////////
INLINE void qpPGWaitBar:: 
set_value(float value) {
  _value = value;
  _bar_state = -1;
}

////////////////////////////////////////////////////////////////////
//     Function: qpPGWaitBar::get_value
//       Access: Published
//  Description: Returns the current value of the bar.
////////////////////////////////////////////////////////////////////
INLINE float qpPGWaitBar:: 
get_value() const {
  return _value;
}

////////////////////////////////////////////////////////////////////
//     Function: qpPGWaitBar::get_percent
//       Access: Published
//  Description: Returns the percentage complete.
////////////////////////////////////////////////////////////////////
INLINE float qpPGWaitBar:: 
get_percent() const {
  return _value / _range * 100.0;
}

////////////////////////////////////////////////////////////////////
//     Function: qpPGWaitBar::set_bar_style
//       Access: Published
//  Description: Sets the kind of frame that is drawn on top of the
//               WaitBar to represent the amount completed.
////////////////////////////////////////////////////////////////////
INLINE void qpPGWaitBar:: 
set_bar_style(const PGFrameStyle &style) {
  _bar_style = style;
  _bar_state = -1;
}

////////////////////////////////////////////////////////////////////
//     Function: qpPGWaitBar::get_bar_style
//       Access: Published
//  Description: Returns the kind of frame that is drawn on top of the
//               WaitBar to represent the amount completed.
////////////////////////////////////////////////////////////////////
INLINE PGFrameStyle qpPGWaitBar:: 
get_bar_style() const {
  return _bar_style;
}
