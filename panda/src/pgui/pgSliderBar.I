// Filename: pgSliderBar.I
// Created by:  masad (19Oct04)
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
//     Function: PGSliderBar::set_range
//       Access: Published
//  Description: Sets the value at which the SliderBar indicates 100%.
////////////////////////////////////////////////////////////////////
INLINE void PGSliderBar:: 
set_range(float range) {
  _range = range;
  _bar_state = -1;
}

////////////////////////////////////////////////////////////////////
//     Function: PGSliderBar::get_range
//       Access: Published
//  Description: Returns the value at which the SliderBar indicates 100%.
////////////////////////////////////////////////////////////////////
INLINE float PGSliderBar:: 
get_range() const {
  return _range;
}

////////////////////////////////////////////////////////////////////
//     Function: PGSliderBar::set_value
//       Access: Published
//  Description: Sets the current value of the bar.  This should range
//               between 0 and get_range().
////////////////////////////////////////////////////////////////////
INLINE void PGSliderBar:: 
set_value(float value) {
  _value = value;
  _bar_state = -1;
}

////////////////////////////////////////////////////////////////////
//     Function: PGSliderBar::get_value
//       Access: Published
//  Description: Returns the current value of the bar.
////////////////////////////////////////////////////////////////////
INLINE float PGSliderBar:: 
get_value() const {
  return _value;
}

////////////////////////////////////////////////////////////////////
//     Function: PGSliderBar::set_speed
//       Access: Published
//  Description: Sets the current speed at which the slider moves
////////////////////////////////////////////////////////////////////
INLINE void PGSliderBar:: 
set_speed(float speed) {
  _speed = speed;
}

////////////////////////////////////////////////////////////////////
//     Function: PGSliderBar::get_speed
//       Access: Published
//  Description: Returns the current speed at which the slider moves.
////////////////////////////////////////////////////////////////////
INLINE float PGSliderBar:: 
get_speed() const {
  return _speed;
}

////////////////////////////////////////////////////////////////////
//     Function: PGSliderBar::get_percent
//       Access: Published
//  Description: Returns the percentage complete.
////////////////////////////////////////////////////////////////////
INLINE float PGSliderBar:: 
get_percent() const {
  return (_value / _range) * 100.0f;
}

////////////////////////////////////////////////////////////////////
//     Function: PGSliderBar::set_bar_style
//       Access: Published
//  Description: Sets the kind of frame that is drawn on top of the
//               SliderBar to represent the amount completed.
////////////////////////////////////////////////////////////////////
INLINE void PGSliderBar:: 
set_bar_style(const PGFrameStyle &style) {
  _bar_style = style;
  _bar_state = -1;
}

////////////////////////////////////////////////////////////////////
//     Function: PGSliderBar::get_bar_style
//       Access: Published
//  Description: Returns the kind of frame that is drawn on top of the
//               SliderBar to represent the amount completed.
////////////////////////////////////////////////////////////////////
INLINE PGFrameStyle PGSliderBar:: 
get_bar_style() const {
  return _bar_style;
}

////////////////////////////////////////////////////////////////////
//     Function: PGSliderBar::get_slider_button
//       Access: Published
//  Description: Returns the slider button that is drawn on the
//               SliderBar to move the slider left or right 
////////////////////////////////////////////////////////////////////
INLINE NodePath PGSliderBar::
get_slider_button() const {
  return _slider_button;
}

////////////////////////////////////////////////////////////////////
//     Function: PGSliderBar::get_left_button
//       Access: Published
//  Description: Returns the left button that is drawn on left of the
//               SliderBar to move the slider left
////////////////////////////////////////////////////////////////////
INLINE NodePath PGSliderBar::
get_left_button() const {
  return _left_button;
}

////////////////////////////////////////////////////////////////////
//     Function: PGSliderBar::get_right_button
//       Access: Published
//  Description: Returns the right button that is drawn on right of the
//               SliderBar to move the slider right
////////////////////////////////////////////////////////////////////
INLINE NodePath PGSliderBar::
get_right_button() const {
  return _right_button;
}

