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

/*
////////////////////////////////////////////////////////////////////
//     Function: PGSliderBar::set_magnifier
//       Access: Published
//  Description: Sets a magnifier value for the mouseWatcherRegion in 
//               the Y direction. 
////////////////////////////////////////////////////////////////////
INLINE void PGSliderBar:: 
set_magnifier(float value) {
  PGItem::_mouseWatcherMagnifier = value;
}
////////////////////////////////////////////////////////////////////
//     Function: PGSliderBar::get_value
//       Access: Published
//  Description: Returns the current value of the bar.
////////////////////////////////////////////////////////////////////
INLINE float PGSliderBar:: 
get_magnifier() const {
  return PGItem::_mouseWatcherMagnifier;
}
*/
////////////////////////////////////////////////////////////////////
//     Function: PGSliderBar::set_value
//       Access: Published
//  Description: Sets the current value of the bar.  This should range
//               between 0 and get_range().
////////////////////////////////////////////////////////////////////
INLINE void PGSliderBar:: 
set_value(float value) {
  _value = value;
  _mapped_value = _negative_mapping ? (_value*0.5 + 0.5) : _value;
  _update_x = _width * (2*_mapped_value - 1);
  _bar_state = -1;
  _update_slider = true;
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
//     Function: PGSliderBar:get_mapped_value
//       Access: Published
//  Description: Returns the _mapped_value which is caluclated from 
//               _value with negative_mapping bool
////////////////////////////////////////////////////////////////////
INLINE float PGSliderBar:: 
get_mapped_value() const {
  return _mapped_value;
}

////////////////////////////////////////////////////////////////////
//     Function: PGSliderBar::get_update_x
//       Access: Published
//  Description: Returns the _update_x which is caluclated from 
//               _mapped_value with width formula
////////////////////////////////////////////////////////////////////
INLINE float PGSliderBar:: 
get_update_x() const {
  return _update_x;
}

////////////////////////////////////////////////////////////////////
//     Function: PGSliderBar::get_update_y
//       Access: Published
//  Description: Returns the _update_y which is caluclated from 
//               _mapped_value with width formula
////////////////////////////////////////////////////////////////////
INLINE float PGSliderBar:: 
get_update_y() const {
  return _update_y;
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
//     Function: PGSliderBar::set_scale
//       Access: Published
//  Description: Sets the scale of the traugh image
////////////////////////////////////////////////////////////////////
INLINE void PGSliderBar:: 
set_scale(float scale) {
  _scale = scale/10;
}

////////////////////////////////////////////////////////////////////
//     Function: PGSliderBar::get_scale
//       Access: Published
//  Description: Returns the scale set on the traugh image
////////////////////////////////////////////////////////////////////
INLINE float PGSliderBar:: 
get_scale() const {
  return _scale;
}

////////////////////////////////////////////////////////////////////
//     Function: PGSliderBar::set_slider_only
//       Access: Published
//  Description: Sets _slider_only. True:only slider button desired.
//               No left or right button to control slider needed.
//               Else, the left and right slider buttons are shown.
////////////////////////////////////////////////////////////////////
INLINE void PGSliderBar:: 
set_slider_only(bool value) {
  _slider_only = value;
}

////////////////////////////////////////////////////////////////////
//     Function: PGSliderBar::get_slider_only
//       Access: Published
//  Description: Returns what type of slider option set
////////////////////////////////////////////////////////////////////
INLINE bool PGSliderBar:: 
get_slider_only() const {
  return _slider_only;
}

////////////////////////////////////////////////////////////////////
//     Function: PGSliderBar::set_negative_mapping
//       Access: Published
//  Description: Sets _negative_mapping. True: slider will receive
//               _value from -1 to +1 else, _value from 0 to + 1
////////////////////////////////////////////////////////////////////
INLINE void PGSliderBar:: 
set_negative_mapping(bool value) {
  _negative_mapping = value;
}

////////////////////////////////////////////////////////////////////
//     Function: PGSliderBar::get_negative_mapping
//       Access: Published
//  Description: Returns what type of value range option set
////////////////////////////////////////////////////////////////////
INLINE bool PGSliderBar:: 
get_negative_mapping() const {
  return _negative_mapping;
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
//     Function: PGSliderBar::get_click_event
//       Access: Published
//  Description: Returns the event name that will be thrown when the
//               slider button is moved
////////////////////////////////////////////////////////////////////
INLINE string PGSliderBar::
get_click_event() const {
  return "updated-slider-" + get_id();
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
//     Function: PGSliderBar::set_slider_button
//       Access: Published
//  Description: sets slider button that is drawn on the
//               SliderBar to move the slider left or right 
////////////////////////////////////////////////////////////////////
INLINE void PGSliderBar::
set_slider_button(NodePath &np, PGSliderButton *button) {
  _slider_button = np;
  _slider = button;
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

