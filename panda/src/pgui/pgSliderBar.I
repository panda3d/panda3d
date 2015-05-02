// Filename: pgSliderBar.I
// Created by:  masad (19Oct04)
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
//     Function: PGSliderBar::set_notify
//       Access: Published
//  Description: Sets the object which will be notified when the
//               PGSliderBar changes.  Set this to NULL to disable
//               this effect.  The PGSliderBar does not retain
//               ownership of the pointer; it is your responsibility
//               to ensure that the notify object does not destruct.
////////////////////////////////////////////////////////////////////
INLINE void PGSliderBar:: 
set_notify(PGSliderBarNotify *notify) {
  PGItem::set_notify(notify);
}

////////////////////////////////////////////////////////////////////
//     Function: PGSliderBar::get_notify
//       Access: Published
//  Description: Returns the object which will be notified when the
//               PGSliderBar changes, if any.  Returns NULL if there
//               is no such object configured.
////////////////////////////////////////////////////////////////////
INLINE PGSliderBarNotify *PGSliderBar:: 
get_notify() const {
  return (PGSliderBarNotify *)PGItem::get_notify();
}

////////////////////////////////////////////////////////////////////
//     Function: PGSliderBar::set_axis
//       Access: Published
//  Description: Specifies the axis of the slider bar's motion.  This
//               should be only one of four vectors: (1, 0, 0), (0, 0,
//               1), (-1, 0, 0), or (0, 0, -1).
//
//               This specifies the vector in which the thumb moves
//               when it is moving from the minimum to the maximum
//               value.
//
//               The axis must be parallel to one of the screen axes,
//               and it must be normalized.  Hence, it may only be one
//               of the above four possibilities; anything else is an
//               error and will result in indeterminate behavior.
//
//               Normally, you should not try to set the axis
//               directly.
////////////////////////////////////////////////////////////////////
INLINE void PGSliderBar:: 
set_axis(const LVector3 &axis) {
  LightReMutexHolder holder(_lock);
  _axis = axis;
  _needs_remanage = true;
  _needs_recompute = true;
}

////////////////////////////////////////////////////////////////////
//     Function: PGSliderBar::get_axis
//       Access: Published
//  Description: Returns the axis of the slider bar's motion.  See
//               set_axis().
////////////////////////////////////////////////////////////////////
INLINE const LVector3 &PGSliderBar:: 
get_axis() const {
  LightReMutexHolder holder(_lock);
  return _axis;
}

////////////////////////////////////////////////////////////////////
//     Function: PGSliderBar::set_range
//       Access: Published
//  Description: Sets the minimum and maxmimum value for the slider.
////////////////////////////////////////////////////////////////////
INLINE void PGSliderBar:: 
set_range(PN_stdfloat min_value, PN_stdfloat max_value) {
  LightReMutexHolder holder(_lock);
  nassertv(min_value != max_value);
  _min_value = min_value;
  _max_value = max_value;
  _needs_recompute = true;

  if (has_notify()) {
    get_notify()->slider_bar_set_range(this);
  }
}

////////////////////////////////////////////////////////////////////
//     Function: PGSliderBar::get_min_value
//       Access: Published
//  Description: Returns the value when the slider is all the way to
//               the left.
////////////////////////////////////////////////////////////////////
INLINE PN_stdfloat PGSliderBar:: 
get_min_value() const {
  LightReMutexHolder holder(_lock);
  return _min_value;
}

////////////////////////////////////////////////////////////////////
//     Function: PGSliderBar::get_max_value
//       Access: Published
//  Description: Returns the value when the slider is all the way to
//               the right.
////////////////////////////////////////////////////////////////////
INLINE PN_stdfloat PGSliderBar:: 
get_max_value() const {
  LightReMutexHolder holder(_lock);
  return _max_value;
}

////////////////////////////////////////////////////////////////////
//     Function: PGSliderBar::set_scroll_size
//       Access: Published
//  Description: Specifies the amount the slider will move when the
//               user clicks on the left or right buttons.
////////////////////////////////////////////////////////////////////
INLINE void PGSliderBar:: 
set_scroll_size(PN_stdfloat value) {
  LightReMutexHolder holder(_lock);
  _scroll_value = value;
  _needs_recompute = true;
}

////////////////////////////////////////////////////////////////////
//     Function: PGSliderBar::get_scroll_size
//       Access: Published
//  Description: Returns the value last set by set_scroll_size().
////////////////////////////////////////////////////////////////////
INLINE PN_stdfloat PGSliderBar:: 
get_scroll_size() const {
  LightReMutexHolder holder(_lock);
  return _scroll_value;
}

////////////////////////////////////////////////////////////////////
//     Function: PGSliderBar::set_page_size
//       Access: Published
//  Description: Specifies the amount of data contained in a single
//               page.  This indicates how much the thumb will jump
//               when the trough is directly clicked; and if
//               resize_thumb is true, it also controls the visible
//               size of the thumb button.
////////////////////////////////////////////////////////////////////
INLINE void PGSliderBar:: 
set_page_size(PN_stdfloat value) {
  LightReMutexHolder holder(_lock);
  _page_value = value;
  _needs_recompute = true;
}

////////////////////////////////////////////////////////////////////
//     Function: PGSliderBar::get_page_size
//       Access: Published
//  Description: Returns the value last set by set_page_size().
////////////////////////////////////////////////////////////////////
INLINE PN_stdfloat PGSliderBar:: 
get_page_size() const {
  LightReMutexHolder holder(_lock);
  return _page_value;
}

////////////////////////////////////////////////////////////////////
//     Function: PGSliderBar::set_value
//       Access: Published
//  Description: Sets the current value of the slider
//               programmatically.  This should range between
//               get_min_value() and get_max_value().
////////////////////////////////////////////////////////////////////
INLINE void PGSliderBar:: 
set_value(PN_stdfloat value) {
  LightReMutexHolder holder(_lock);
  set_ratio((value - _min_value) / (_max_value - _min_value));
}

////////////////////////////////////////////////////////////////////
//     Function: PGSliderBar::get_value
//       Access: Published
//  Description: Returns the current value of the slider.
////////////////////////////////////////////////////////////////////
INLINE PN_stdfloat PGSliderBar:: 
get_value() const {
  LightReMutexHolder holder(_lock);
  return get_ratio() * (_max_value - _min_value) + _min_value;
}

////////////////////////////////////////////////////////////////////
//     Function: PGSliderBar::set_ratio
//       Access: Published
//  Description: Sets the current value of the slider, expressed in
//               the range 0 .. 1.
////////////////////////////////////////////////////////////////////
INLINE void PGSliderBar:: 
set_ratio(PN_stdfloat ratio) {
  LightReMutexHolder holder(_lock);
  if (!is_button_down()) {
    internal_set_ratio(ratio);
  }
}

////////////////////////////////////////////////////////////////////
//     Function: PGSliderBar::get_ratio
//       Access: Published
//  Description: Returns the current value of the slider, expressed in
//               the range 0 .. 1.
////////////////////////////////////////////////////////////////////
INLINE PN_stdfloat PGSliderBar:: 
get_ratio() const {
  LightReMutexHolder holder(_lock);
  return _ratio;
}

////////////////////////////////////////////////////////////////////
//     Function: PGSliderBar::is_button_down
//       Access: Published
//  Description: Returns true if the user is currently holding down
//               the mouse button to manipulate the slider.  When
//               true, calls to set_ratio() or set_value() will have
//               no effect.
////////////////////////////////////////////////////////////////////
INLINE bool PGSliderBar:: 
is_button_down() const {
  LightReMutexHolder holder(_lock);
  return _dragging || _mouse_button_page || 
    (_scroll_button_held != (PGItem *)NULL);
}

////////////////////////////////////////////////////////////////////
//     Function: PGSliderBar::set_resize_thumb
//       Access: Published
//  Description: Sets the resize_thumb flag.  When this is true, the
//               thumb button's frame will be adjusted so that its
//               width visually represents the page size.  When this
//               is false, the thumb button will be left alone.
////////////////////////////////////////////////////////////////////
INLINE void PGSliderBar:: 
set_resize_thumb(bool resize_thumb) {
  LightReMutexHolder holder(_lock);
  _resize_thumb = resize_thumb;
  _needs_recompute = true;
}

////////////////////////////////////////////////////////////////////
//     Function: PGSliderBar::get_resize_thumb
//       Access: Published
//  Description: Returns the resize_thumb flag.  See
//               set_resize_thumb().
////////////////////////////////////////////////////////////////////
INLINE bool PGSliderBar:: 
get_resize_thumb() const {
  LightReMutexHolder holder(_lock);
  return _resize_thumb;
}

////////////////////////////////////////////////////////////////////
//     Function: PGSliderBar::set_manage_pieces
//       Access: Published
//  Description: Sets the manage_pieces flag.  When this is true, the
//               sub-pieces of the slider bar--that is, the thumb, and
//               the left and right scroll buttons--are automatically
//               positioned and/or resized when the slider bar's
//               overall frame is changed.
////////////////////////////////////////////////////////////////////
INLINE void PGSliderBar:: 
set_manage_pieces(bool manage_pieces) {
  LightReMutexHolder holder(_lock);
  _manage_pieces = manage_pieces;
  _needs_remanage = true;
  _needs_recompute = true;
}

////////////////////////////////////////////////////////////////////
//     Function: PGSliderBar::get_manage_pieces
//       Access: Published
//  Description: Returns the manage_pieces flag.  See
//               set_manage_pieces().
////////////////////////////////////////////////////////////////////
INLINE bool PGSliderBar:: 
get_manage_pieces() const {
  LightReMutexHolder holder(_lock);
  return _manage_pieces;
}

////////////////////////////////////////////////////////////////////
//     Function: PGSliderBar::set_thumb_button
//       Access: Published
//  Description: Sets the PGButton object that will serve as the thumb
//               for this slider.  This button visually represents the
//               position of the slider, and can be dragged left and
//               right by the user.
//
//               It is the responsibility of the caller to ensure that
//               the button object is parented to the PGSliderBar
//               node.
////////////////////////////////////////////////////////////////////
INLINE void PGSliderBar::
set_thumb_button(PGButton *thumb_button) {
  LightReMutexHolder holder(_lock);
  if (_thumb_button != (PGButton *)NULL) {
    _thumb_button->set_notify(NULL);
  }
  _thumb_button = thumb_button;
  if (_thumb_button != (PGButton *)NULL) {
    _thumb_button->set_notify(this);
  }
  _needs_remanage = true;
  _needs_recompute = true;
}

////////////////////////////////////////////////////////////////////
//     Function: PGSliderBar::clear_thumb_button
//       Access: Published
//  Description: Removes the thumb button object from control of the
//               frame.  It is your responsibility to actually remove
//               or hide the button itself.
////////////////////////////////////////////////////////////////////
INLINE void PGSliderBar::
clear_thumb_button() {
  set_thumb_button(NULL);
}

////////////////////////////////////////////////////////////////////
//     Function: PGSliderBar::get_thumb_button
//       Access: Published
//  Description: Returns the PGButton that serves as the thumb for
//               this slider, or NULL if it is not set.
////////////////////////////////////////////////////////////////////
INLINE PGButton *PGSliderBar::
get_thumb_button() const {
  LightReMutexHolder holder(_lock);
  return _thumb_button;
}

////////////////////////////////////////////////////////////////////
//     Function: PGSliderBar::set_left_button
//       Access: Published
//  Description: Sets the PGButton object that will serve as the left
//               scroll button for this slider.  This button is
//               optional; if present, the user can click on it to
//               move scroll_size units at a time to the left.
//
//               It is the responsibility of the caller to ensure that
//               the button object is parented to the PGSliderBar
//               node.
////////////////////////////////////////////////////////////////////
INLINE void PGSliderBar::
set_left_button(PGButton *left_button) {
  LightReMutexHolder holder(_lock);
  if (_left_button != (PGButton *)NULL) {
    _left_button->set_notify(NULL);
  }
  _left_button = left_button;
  if (_left_button != (PGButton *)NULL) {
    _left_button->set_notify(this);
  }
  _needs_remanage = true;
  _needs_recompute = true;
}

////////////////////////////////////////////////////////////////////
//     Function: PGSliderBar::clear_left_button
//       Access: Published
//  Description: Removes the left button object from control of the
//               frame.  It is your responsibility to actually remove
//               or hide the button itself.
////////////////////////////////////////////////////////////////////
INLINE void PGSliderBar::
clear_left_button() {
  set_left_button(NULL);
}

////////////////////////////////////////////////////////////////////
//     Function: PGSliderBar::get_left_button
//       Access: Published
//  Description: Returns the PGButton that serves as the left scroll
//               button for this slider, if any, or NULL if it is not
//               set.
////////////////////////////////////////////////////////////////////
INLINE PGButton *PGSliderBar::
get_left_button() const {
  LightReMutexHolder holder(_lock);
  return _left_button;
}

////////////////////////////////////////////////////////////////////
//     Function: PGSliderBar::set_right_button
//       Access: Published
//  Description: Sets the PGButton object that will serve as the right
//               scroll button for this slider.  This button is
//               optional; if present, the user can click on it to
//               move scroll_size units at a time to the right.
//
//               It is the responsibility of the caller to ensure that
//               the button object is parented to the PGSliderBar
//               node.
////////////////////////////////////////////////////////////////////
INLINE void PGSliderBar::
set_right_button(PGButton *right_button) {
  LightReMutexHolder holder(_lock);
  if (_right_button != (PGButton *)NULL) {
    _right_button->set_notify(NULL);
  }
  _right_button = right_button;
  if (_right_button != (PGButton *)NULL) {
    _right_button->set_notify(this);
  }
  _needs_remanage = true;
  _needs_recompute = true;
}

////////////////////////////////////////////////////////////////////
//     Function: PGSliderBar::clear_right_button
//       Access: Published
//  Description: Removes the right button object from control of the
//               frame.  It is your responsibility to actually remove
//               or hide the button itself.
////////////////////////////////////////////////////////////////////
INLINE void PGSliderBar::
clear_right_button() {
  set_right_button(NULL);
}

////////////////////////////////////////////////////////////////////
//     Function: PGSliderBar::get_right_button
//       Access: Published
//  Description: Returns the PGButton that serves as the right scroll
//               button for this slider, if any, or NULL if it is not
//               set.
////////////////////////////////////////////////////////////////////
INLINE PGButton *PGSliderBar::
get_right_button() const {
  LightReMutexHolder holder(_lock);
  return _right_button;
}

////////////////////////////////////////////////////////////////////
//     Function: PGSliderBar::get_adjust_prefix
//       Access: Published, Static
//  Description: Returns the prefix that is used to define the adjust
//               event for all PGSliderBars.  The adjust event is the
//               concatenation of this string followed by get_id().
////////////////////////////////////////////////////////////////////
INLINE string PGSliderBar::
get_adjust_prefix() {
  return "adjust-";
}

////////////////////////////////////////////////////////////////////
//     Function: PGSliderBar::get_adjust_event
//       Access: Published
//  Description: Returns the event name that will be thrown when the
//               slider bar value is adjusted by the user or
//               programmatically.
////////////////////////////////////////////////////////////////////
INLINE string PGSliderBar::
get_adjust_event() const {
  LightReMutexHolder holder(_lock);
  return get_adjust_prefix() + get_id();
}

////////////////////////////////////////////////////////////////////
//     Function: PGSliderBar::internal_set_ratio
//       Access: Private
//  Description: Sets the current value of the slider, expressed in
//               the range 0 .. 1, without checking whether the user
//               is currently manipulating the slider.
////////////////////////////////////////////////////////////////////
INLINE void PGSliderBar:: 
internal_set_ratio(PN_stdfloat ratio) {
  _ratio = max(min(ratio, (PN_stdfloat)1.0), (PN_stdfloat)0.0);
  _needs_reposition = true;
  adjust();
}
