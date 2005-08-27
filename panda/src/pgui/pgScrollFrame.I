// Filename: pgScrollFrame.I
// Created by:  drose (17Aug05)
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
//     Function: PGScrollFrame::set_virtual_frame
//       Access: Published
//  Description: Sets the bounding rectangle of the virtual frame.
//               This is the size of the large, virtual canvas which
//               we can see only a portion of at any given time.
////////////////////////////////////////////////////////////////////
INLINE void PGScrollFrame::
set_virtual_frame(float left, float right, float bottom, float top) {
  set_virtual_frame(LVecBase4f(left, right, bottom, top));
}

////////////////////////////////////////////////////////////////////
//     Function: PGScrollFrame::set_virtual_frame
//       Access: Published
//  Description: Sets the bounding rectangle of the virtual frame.
//               This is the size of the large, virtual canvas which
//               we can see only a portion of at any given time.
////////////////////////////////////////////////////////////////////
INLINE void PGScrollFrame::
set_virtual_frame(const LVecBase4f &frame) {
  _has_virtual_frame = true;
  _virtual_frame = frame;

  if (_auto_hide) {
    _needs_remanage = true;
  }
  _needs_recompute_clip = true;
}

////////////////////////////////////////////////////////////////////
//     Function: PGScrollFrame::get_virtual_frame
//       Access: Published
//  Description: Returns the bounding rectangle of the virtual frame.
//               See set_virtual_frame().  If has_virtual_frame() is
//               false, this returns the item's clip frame.
////////////////////////////////////////////////////////////////////
INLINE const LVecBase4f &PGScrollFrame::
get_virtual_frame() const {
  return _has_virtual_frame ? _virtual_frame : get_clip_frame();
}

////////////////////////////////////////////////////////////////////
//     Function: PGScrollFrame::has_virtual_frame
//       Access: Published
//  Description: Returns true if the virtual frame has a bounding
//               rectangle; see set_virtual_frame().  Most
//               PGScrollFrame objects will have a virtual frame.
////////////////////////////////////////////////////////////////////
INLINE bool PGScrollFrame::
has_virtual_frame() const {
  return _has_virtual_frame;
}

////////////////////////////////////////////////////////////////////
//     Function: PGScrollFrame::clear_virtual_frame
//       Access: Published
//  Description: Removes the virtual frame from the item.  This
//               effectively sets the virtual frame to the same size
//               as the clip frame.  Scrolling will no longer be
//               possible.
////////////////////////////////////////////////////////////////////
INLINE void PGScrollFrame::
clear_virtual_frame() {
  _has_virtual_frame = false;
}

////////////////////////////////////////////////////////////////////
//     Function: PGScrollFrame::set_manage_pieces
//       Access: Published
//  Description: Sets the manage_pieces flag.  When this is true, the
//               sub-pieces of the scroll frame--that is, the two
//               scroll bars--are automatically positioned and/or
//               resized when the scroll frame's overall frame is
//               changed.  They are also automatically resized to fill
//               in the gap when one or the other is hidden.
////////////////////////////////////////////////////////////////////
INLINE void PGScrollFrame:: 
set_manage_pieces(bool manage_pieces) {
  _manage_pieces = manage_pieces;
  _needs_remanage = true;
  _needs_recompute_clip = true;
}

////////////////////////////////////////////////////////////////////
//     Function: PGScrollFrame::get_manage_pieces
//       Access: Published
//  Description: Returns the manage_pieces flag.  See
//               set_manage_pieces().
////////////////////////////////////////////////////////////////////
INLINE bool PGScrollFrame:: 
get_manage_pieces() const {
  return _manage_pieces;
}

////////////////////////////////////////////////////////////////////
//     Function: PGScrollFrame::set_auto_hide
//       Access: Published
//  Description: Sets the auto_hide flag.  When this is true, the
//               two scroll bars are automatically hidden if they are
//               not needed (that is, if the virtual frame would fit
//               within the clip frame without them), and they are
//               automatically shown when they are needed.
//
//               Setting this flag true forces the manage_pieces flag
//               to also be set true.
////////////////////////////////////////////////////////////////////
INLINE void PGScrollFrame:: 
set_auto_hide(bool auto_hide) {
  _auto_hide = auto_hide;
  if (_auto_hide) {
    set_manage_pieces(true);
    _needs_remanage = true;
  }
}

////////////////////////////////////////////////////////////////////
//     Function: PGScrollFrame::get_auto_hide
//       Access: Published
//  Description: Returns the auto_hide flag.  See
//               set_auto_hide().
////////////////////////////////////////////////////////////////////
INLINE bool PGScrollFrame:: 
get_auto_hide() const {
  return _auto_hide;
}

////////////////////////////////////////////////////////////////////
//     Function: PGScrollFrame::set_horizontal_slider
//       Access: Published
//  Description: Sets the PGSliderBar object that will serve as the
//               horizontal scroll bar for this frame.  It is your
//               responsibility to parent this slider bar to the frame
//               and move it to the appropriate place.
////////////////////////////////////////////////////////////////////
INLINE void PGScrollFrame::
set_horizontal_slider(PGSliderBar *horizontal_slider) {
  if (_horizontal_slider != (PGSliderBar *)NULL) {
    _horizontal_slider->set_notify(NULL);
  }
  _horizontal_slider = horizontal_slider;
  if (_horizontal_slider != (PGSliderBar *)NULL) {
    _horizontal_slider->set_notify(this);
  }
  _needs_recompute_clip = true;
}

////////////////////////////////////////////////////////////////////
//     Function: PGScrollFrame::clear_horizontal_slider
//       Access: Published
//  Description: Removes the horizontal scroll bar from control of the
//               frame.  It is your responsibility to actually remove
//               or hide the object itself.
////////////////////////////////////////////////////////////////////
INLINE void PGScrollFrame::
clear_horizontal_slider() {
  set_horizontal_slider(NULL);
}

////////////////////////////////////////////////////////////////////
//     Function: PGScrollFrame::get_horizontal_slider
//       Access: Published
//  Description: Returns the PGSliderBar that serves as the horizontal
//               scroll bar for this frame, if any, or NULL if it is
//               not set.
////////////////////////////////////////////////////////////////////
INLINE PGSliderBar *PGScrollFrame::
get_horizontal_slider() const {
  return _horizontal_slider;
}

////////////////////////////////////////////////////////////////////
//     Function: PGScrollFrame::set_vertical_slider
//       Access: Published
//  Description: Sets the PGSliderBar object that will serve as the
//               vertical scroll bar for this frame.  It is your
//               responsibility to parent this slider bar to the frame
//               and move it to the appropriate place.
////////////////////////////////////////////////////////////////////
INLINE void PGScrollFrame::
set_vertical_slider(PGSliderBar *vertical_slider) {
  if (_vertical_slider != (PGSliderBar *)NULL) {
    _vertical_slider->set_notify(NULL);
  }
  _vertical_slider = vertical_slider;
  if (_vertical_slider != (PGSliderBar *)NULL) {
    _vertical_slider->set_notify(this);
  }
  _needs_recompute_clip = true;
}

////////////////////////////////////////////////////////////////////
//     Function: PGScrollFrame::clear_vertical_slider
//       Access: Published
//  Description: Removes the vertical scroll bar from control of the
//               frame.  It is your responsibility to actually remove
//               or hide the object itself.
////////////////////////////////////////////////////////////////////
INLINE void PGScrollFrame::
clear_vertical_slider() {
  set_vertical_slider(NULL);
}

////////////////////////////////////////////////////////////////////
//     Function: PGScrollFrame::get_vertical_slider
//       Access: Published
//  Description: Returns the PGSliderBar that serves as the vertical
//               scroll bar for this frame, if any, or NULL if it is
//               not set.
////////////////////////////////////////////////////////////////////
INLINE PGSliderBar *PGScrollFrame::
get_vertical_slider() const {
  return _vertical_slider;
}

////////////////////////////////////////////////////////////////////
//     Function: PGScrollFrame::recompute
//       Access: Published
//  Description: Forces the PGScrollFrame to recompute itself right
//               now.  Normally this should not be required.
////////////////////////////////////////////////////////////////////
INLINE void PGScrollFrame::
recompute() {
  recompute_clip();
  recompute_canvas();
}
