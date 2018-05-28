/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file pgScrollFrame.I
 * @author drose
 * @date 2005-08-17
 */

/**
 * Sets the bounding rectangle of the virtual frame.  This is the size of the
 * large, virtual canvas which we can see only a portion of at any given time.
 */
INLINE void PGScrollFrame::
set_virtual_frame(PN_stdfloat left, PN_stdfloat right, PN_stdfloat bottom, PN_stdfloat top) {
  set_virtual_frame(LVecBase4(left, right, bottom, top));
}

/**
 * Sets the bounding rectangle of the virtual frame.  This is the size of the
 * large, virtual canvas which we can see only a portion of at any given time.
 */
INLINE void PGScrollFrame::
set_virtual_frame(const LVecBase4 &frame) {
  LightReMutexHolder holder(_lock);
  _has_virtual_frame = true;
  _virtual_frame = frame;

  if (_auto_hide) {
    _needs_remanage = true;
  }
  _needs_recompute_clip = true;
}

/**
 * Returns the bounding rectangle of the virtual frame.  See
 * set_virtual_frame().  If has_virtual_frame() is false, this returns the
 * item's clip frame.
 */
INLINE const LVecBase4 &PGScrollFrame::
get_virtual_frame() const {
  LightReMutexHolder holder(_lock);
  return _has_virtual_frame ? _virtual_frame : get_clip_frame();
}

/**
 * Returns true if the virtual frame has a bounding rectangle; see
 * set_virtual_frame().  Most PGScrollFrame objects will have a virtual frame.
 */
INLINE bool PGScrollFrame::
has_virtual_frame() const {
  LightReMutexHolder holder(_lock);
  return _has_virtual_frame;
}

/**
 * Removes the virtual frame from the item.  This effectively sets the virtual
 * frame to the same size as the clip frame.  Scrolling will no longer be
 * possible.
 */
INLINE void PGScrollFrame::
clear_virtual_frame() {
  LightReMutexHolder holder(_lock);
  _has_virtual_frame = false;
}

/**
 * Sets the manage_pieces flag.  When this is true, the sub-pieces of the
 * scroll frame--that is, the two scroll bars--are automatically positioned
 * and/or resized when the scroll frame's overall frame is changed.  They are
 * also automatically resized to fill in the gap when one or the other is
 * hidden.
 */
INLINE void PGScrollFrame::
set_manage_pieces(bool manage_pieces) {
  LightReMutexHolder holder(_lock);
  _manage_pieces = manage_pieces;
  _needs_remanage = true;
  _needs_recompute_clip = true;
}

/**
 * Returns the manage_pieces flag.  See set_manage_pieces().
 */
INLINE bool PGScrollFrame::
get_manage_pieces() const {
  LightReMutexHolder holder(_lock);
  return _manage_pieces;
}

/**
 * Sets the auto_hide flag.  When this is true, the two scroll bars are
 * automatically hidden if they are not needed (that is, if the virtual frame
 * would fit within the clip frame without them), and they are automatically
 * shown when they are needed.
 *
 * Setting this flag true forces the manage_pieces flag to also be set true.
 */
INLINE void PGScrollFrame::
set_auto_hide(bool auto_hide) {
  LightReMutexHolder holder(_lock);
  _auto_hide = auto_hide;
  if (_auto_hide) {
    set_manage_pieces(true);
    _needs_remanage = true;
  }
}

/**
 * Returns the auto_hide flag.  See set_auto_hide().
 */
INLINE bool PGScrollFrame::
get_auto_hide() const {
  LightReMutexHolder holder(_lock);
  return _auto_hide;
}

/**
 * Sets the PGSliderBar object that will serve as the horizontal scroll bar
 * for this frame.  It is your responsibility to parent this slider bar to the
 * frame and move it to the appropriate place.
 */
INLINE void PGScrollFrame::
set_horizontal_slider(PGSliderBar *horizontal_slider) {
  LightReMutexHolder holder(_lock);
  if (_horizontal_slider != nullptr) {
    _horizontal_slider->set_notify(nullptr);
  }
  _horizontal_slider = horizontal_slider;
  if (_horizontal_slider != nullptr) {
    _horizontal_slider->set_notify(this);
  }
  _needs_recompute_clip = true;
}

/**
 * Removes the horizontal scroll bar from control of the frame.  It is your
 * responsibility to actually remove or hide the object itself.
 */
INLINE void PGScrollFrame::
clear_horizontal_slider() {
  set_horizontal_slider(nullptr);
}

/**
 * Returns the PGSliderBar that serves as the horizontal scroll bar for this
 * frame, if any, or NULL if it is not set.
 */
INLINE PGSliderBar *PGScrollFrame::
get_horizontal_slider() const {
  LightReMutexHolder holder(_lock);
  return _horizontal_slider;
}

/**
 * Sets the PGSliderBar object that will serve as the vertical scroll bar for
 * this frame.  It is your responsibility to parent this slider bar to the
 * frame and move it to the appropriate place.
 */
INLINE void PGScrollFrame::
set_vertical_slider(PGSliderBar *vertical_slider) {
  LightReMutexHolder holder(_lock);
  if (_vertical_slider != nullptr) {
    _vertical_slider->set_notify(nullptr);
  }
  _vertical_slider = vertical_slider;
  if (_vertical_slider != nullptr) {
    _vertical_slider->set_notify(this);
  }
  _needs_recompute_clip = true;
}

/**
 * Removes the vertical scroll bar from control of the frame.  It is your
 * responsibility to actually remove or hide the object itself.
 */
INLINE void PGScrollFrame::
clear_vertical_slider() {
  set_vertical_slider(nullptr);
}

/**
 * Returns the PGSliderBar that serves as the vertical scroll bar for this
 * frame, if any, or NULL if it is not set.
 */
INLINE PGSliderBar *PGScrollFrame::
get_vertical_slider() const {
  LightReMutexHolder holder(_lock);
  return _vertical_slider;
}

/**
 * Forces the PGScrollFrame to recompute itself right now.  Normally this
 * should not be required.
 */
INLINE void PGScrollFrame::
recompute() {
  LightReMutexHolder holder(_lock);
  recompute_clip();
  recompute_canvas();
}
