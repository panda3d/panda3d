// Filename: pgItem.I
// Created by:  drose (02Jul01)
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
//     Function: PGItem::get_region
//       Access: Public
//  Description: Returns the MouseWatcherRegion associated with this
//               item.  Every PGItem has a MouseWatcherRegion
//               associated with it, that is created when the PGItem
//               is created; it does not change during the lifetime of
//               the PGItem.  Even items that do not have a frame have
//               an associated MouseWatcherRegion, although it will
//               not be used in this case.
////////////////////////////////////////////////////////////////////
INLINE PGMouseWatcherRegion *PGItem:: 
get_region() const {
  return _region;
}

////////////////////////////////////////////////////////////////////
//     Function: PGItem::set_frame
//       Access: Published
//  Description: Sets the bounding rectangle of the item, in local
//               coordinates.  This is the region on screen within
//               which the mouse will be considered to be within the
//               item.  Normally, it should correspond to the bounding
//               rectangle of the visible geometry of the item.
////////////////////////////////////////////////////////////////////
INLINE void PGItem::
set_frame(float left, float right, float bottom, float top) {
  set_frame(LVecBase4f(left, right, bottom, top));
}

////////////////////////////////////////////////////////////////////
//     Function: PGItem::set_frame
//       Access: Published
//  Description: Sets the bounding rectangle of the item, in local
//               coordinates.  This is the region on screen within
//               which the mouse will be considered to be within the
//               item.  Normally, it should correspond to the bounding
//               rectangle of the visible geometry of the item.
////////////////////////////////////////////////////////////////////
INLINE void PGItem::
set_frame(const LVecBase4f &frame) {
  _has_frame = true;
  _frame = frame;
  mark_frames_stale();
}

////////////////////////////////////////////////////////////////////
//     Function: PGItem::get_frame
//       Access: Published
//  Description: Returns the bounding rectangle of the item.  See
//               set_frame().  It is an error to call this if
//               has_frame() returns false.
////////////////////////////////////////////////////////////////////
INLINE const LVecBase4f &PGItem::
get_frame() const {
  nassertr(has_frame(), _frame);
  return _frame;
}

////////////////////////////////////////////////////////////////////
//     Function: PGItem::has_frame
//       Access: Published
//  Description: Returns true if the item has a bounding rectangle;
//               see set_frame().
////////////////////////////////////////////////////////////////////
INLINE bool PGItem::
has_frame() const {
  return _has_frame;
}

////////////////////////////////////////////////////////////////////
//     Function: PGItem::clear_frame
//       Access: Published
//  Description: Removes the bounding rectangle from the item.  It
//               will no longer be possible to position the mouse
//               within the item; see set_frame().
////////////////////////////////////////////////////////////////////
INLINE void PGItem::
clear_frame() {
  _has_frame = false;
  mark_frames_stale();
}

////////////////////////////////////////////////////////////////////
//     Function: PGItem::set_state
//       Access: Published
//  Description: Sets the "state" of this particular PGItem.  
//
//               The PGItem node will render as if it were the
//               subgraph assigned to the corresponding index via
//               set_state_def().
////////////////////////////////////////////////////////////////////
INLINE void PGItem::
set_state(int state) {
  _state = state;
}

////////////////////////////////////////////////////////////////////
//     Function: PGItem::get_state
//       Access: Published
//  Description: Returns the "state" of this particular PGItem.  See
//               set_state().
////////////////////////////////////////////////////////////////////
INLINE int PGItem::
get_state() const {
  return _state;
}

////////////////////////////////////////////////////////////////////
//     Function: PGItem::set_active
//       Access: Published
//  Description: Sets whether the PGItem is active for mouse watching.
//               This is not necessarily related to the
//               active/inactive appearance of the item, which is
//               controlled by set_state(), but it does affect whether
//               it responds to mouse events.
////////////////////////////////////////////////////////////////////
INLINE void PGItem::
set_active(bool active) {
  _active = active;
}

////////////////////////////////////////////////////////////////////
//     Function: PGItem::get_active
//       Access: Published
//  Description: Returns whether the PGItem is currently active for
//               mouse events.  See set_active().
////////////////////////////////////////////////////////////////////
INLINE bool PGItem::
get_active() const {
  return _active;
}

////////////////////////////////////////////////////////////////////
//     Function: PGItem::get_id
//       Access: Published
//  Description: Returns the unique ID assigned to this PGItem.  This
//               will be assigned to the region created with the
//               MouseWatcher, and will thus be used to generate event
//               names.
////////////////////////////////////////////////////////////////////
INLINE const string &PGItem::
get_id() const {
  return _region->get_name();
}

////////////////////////////////////////////////////////////////////
//     Function: PGItem::get_enter_event
//       Access: Published
//  Description: Returns the event name that will be thrown when the
//               item is active and the mouse enters its frame.
////////////////////////////////////////////////////////////////////
INLINE string PGItem::
get_enter_event() const {
  return "enter-" + get_id();
}

////////////////////////////////////////////////////////////////////
//     Function: PGItem::get_exit_event
//       Access: Published
//  Description: Returns the event name that will be thrown when the
//               item is active and the mouse exits its frame.
////////////////////////////////////////////////////////////////////
INLINE string PGItem::
get_exit_event() const {
  return "exit-" + get_id();
}

////////////////////////////////////////////////////////////////////
//     Function: PGItem::get_button_down_event
//       Access: Published
//  Description: Returns the event name that will be thrown when the
//               item is active and a mouse or keyboard button is
//               depressed while the mouse is within the frame.
//
//               This event will be thrown with one parameter, which
//               will be the name of the button that generated the
//               event.
////////////////////////////////////////////////////////////////////
INLINE string PGItem::
get_button_down_event() const {
  return "bdown-" + get_id();
}

////////////////////////////////////////////////////////////////////
//     Function: PGItem::get_button_up_event
//       Access: Published
//  Description: Returns the event name that will be thrown when the
//               item is active and a mouse or keyboard button,
//               formerly clicked down is within the frame, is
//               released.
//
//               This event will be thrown with two parameters, the
//               first of which will be the name of the button that
//               generated the event, and the second of which will be
//               a boolean flag indicating true if the mouse was
//               within the frame while the button was released.
////////////////////////////////////////////////////////////////////
INLINE string PGItem::
get_button_up_event() const {
  return "bup-" + get_id();
}

////////////////////////////////////////////////////////////////////
//     Function: PGItem::set_text_node
//       Access: Published, Static
//  Description: Changes the TextNode object that will be used by all
//               PGItems to generate default labels given a string.
//               This can be loaded with the default font, etc.
////////////////////////////////////////////////////////////////////
INLINE void PGItem::
set_text_node(TextNode *node) {
  _text_node = node;
}
