// Filename: pgItem.I
// Created by:  drose (13Mar02)
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
//     Function: PGItem::get_active
//       Access: Published
//  Description: Returns whether the PGItem is currently active for
//               mouse events.  See set_active().
////////////////////////////////////////////////////////////////////
INLINE bool PGItem::
get_active() const {
  return (_flags & F_active) != 0;
}

////////////////////////////////////////////////////////////////////
//     Function: PGItem::get_focus
//       Access: Published
//  Description: Returns whether the PGItem currently has focus for
//               keyboard events.  See set_focus().
////////////////////////////////////////////////////////////////////
INLINE bool PGItem::
get_focus() const {
  return (_flags & F_focus) != 0;
}

////////////////////////////////////////////////////////////////////
//     Function: PGItem::get_background_focus
//       Access: Published
//  Description: Returns whether background_focus is currently
//               enabled.  See set_background_focus().
////////////////////////////////////////////////////////////////////
INLINE bool PGItem::
get_background_focus() const {
  return (_flags & F_background_focus) != 0;
}

////////////////////////////////////////////////////////////////////
//     Function: PGItem::set_suppress_flags
//       Access: Published
//  Description: This is just an interface to set the suppress flags
//               on the underlying MouseWatcherRegion.  See
//               MouseWatcherRegion::set_suppress_flags().
////////////////////////////////////////////////////////////////////
INLINE void PGItem::
set_suppress_flags(int suppress_flags) {
  _region->set_suppress_flags(suppress_flags);
}

////////////////////////////////////////////////////////////////////
//     Function: PGItem::get_suppress_flags
//       Access: Published
//  Description: This is just an interface to get the suppress flags
//               on the underlying MouseWatcherRegion.  See
//               MouseWatcherRegion::get_suppress_flags().
////////////////////////////////////////////////////////////////////
INLINE int PGItem::
get_suppress_flags() const {
  return _region->get_suppress_flags();
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
//     Function: PGItem::set_id
//       Access: Published
//  Description: Set the unique ID assigned to this PGItem.  It is the
//               user's responsibility to ensure that this ID is
//               unique.
//
//               Normally, this should not need to be called, as the
//               PGItem will assign itself an ID when it is created,
//               but this function allows the user to decide to
//               redefine the ID to be something possibly more
//               meaningful.
////////////////////////////////////////////////////////////////////
INLINE void PGItem::
set_id(const string &id) {
  _region->set_name(id);
}

////////////////////////////////////////////////////////////////////
//     Function: PGItem::get_enter_prefix
//       Access: Published, Static
//  Description: Returns the prefix that is used to define the enter
//               event for all PGItems.  The enter event is the
//               concatenation of this string followed by get_id().
////////////////////////////////////////////////////////////////////
INLINE string PGItem::
get_enter_prefix() {
  return "enter-";
}

////////////////////////////////////////////////////////////////////
//     Function: PGItem::get_exit_prefix
//       Access: Published, Static
//  Description: Returns the prefix that is used to define the exit
//               event for all PGItems.  The exit event is the
//               concatenation of this string followed by get_id().
////////////////////////////////////////////////////////////////////
INLINE string PGItem::
get_exit_prefix() {
  return "exit-";
}

////////////////////////////////////////////////////////////////////
//     Function: PGItem::get_within_prefix
//       Access: Published, Static
//  Description: Returns the prefix that is used to define the within
//               event for all PGItems.  The within event is the
//               concatenation of this string followed by get_id().
////////////////////////////////////////////////////////////////////
INLINE string PGItem::
get_within_prefix() {
  return "within-";
}

////////////////////////////////////////////////////////////////////
//     Function: PGItem::get_without_prefix
//       Access: Published, Static
//  Description: Returns the prefix that is used to define the without
//               event for all PGItems.  The without event is the
//               concatenation of this string followed by get_id().
////////////////////////////////////////////////////////////////////
INLINE string PGItem::
get_without_prefix() {
  return "without-";
}

////////////////////////////////////////////////////////////////////
//     Function: PGItem::get_focus_in_prefix
//       Access: Published, Static
//  Description: Returns the prefix that is used to define the focus_in
//               event for all PGItems.  The focus_in event is the
//               concatenation of this string followed by get_id().
//
//               Unlike most item events, this event is thrown with no
//               parameters.
////////////////////////////////////////////////////////////////////
INLINE string PGItem::
get_focus_in_prefix() {
  return "fin-";
}

////////////////////////////////////////////////////////////////////
//     Function: PGItem::get_focus_out_prefix
//       Access: Published, Static
//  Description: Returns the prefix that is used to define the focus_out
//               event for all PGItems.  The focus_out event is the
//               concatenation of this string followed by get_id().
//
//               Unlike most item events, this event is thrown with no
//               parameters.
////////////////////////////////////////////////////////////////////
INLINE string PGItem::
get_focus_out_prefix() {
  return "fout-";
}

////////////////////////////////////////////////////////////////////
//     Function: PGItem::get_press_prefix
//       Access: Published, Static
//  Description: Returns the prefix that is used to define the press
//               event for all PGItems.  The press event is the
//               concatenation of this string followed by a button
//               name, followed by a hyphen and get_id().
////////////////////////////////////////////////////////////////////
INLINE string PGItem::
get_press_prefix() {
  return "press-";
}

////////////////////////////////////////////////////////////////////
//     Function: PGItem::get_release_prefix
//       Access: Published, Static
//  Description: Returns the prefix that is used to define the release
//               event for all PGItems.  The release event is the
//               concatenation of this string followed by a button
//               name, followed by a hyphen and get_id().
////////////////////////////////////////////////////////////////////
INLINE string PGItem::
get_release_prefix() {
  return "release-";
}

////////////////////////////////////////////////////////////////////
//     Function: PGItem::get_keystroke_prefix
//       Access: Published, Static
//  Description: Returns the prefix that is used to define the
//               keystroke event for all PGItems.  The keystroke event
//               is the concatenation of this string followed by a
//               hyphen and get_id().
////////////////////////////////////////////////////////////////////
INLINE string PGItem::
get_keystroke_prefix() {
  return "keystroke-";
}

////////////////////////////////////////////////////////////////////
//     Function: PGItem::get_enter_event
//       Access: Published
//  Description: Returns the event name that will be thrown when the
//               item is active and the mouse enters its frame, but
//               not any nested frames.
////////////////////////////////////////////////////////////////////
INLINE string PGItem::
get_enter_event() const {
  return get_enter_prefix() + get_id();
}

////////////////////////////////////////////////////////////////////
//     Function: PGItem::get_exit_event
//       Access: Published
//  Description: Returns the event name that will be thrown when the
//               item is active and the mouse exits its frame, or
//               enters a nested frame.
////////////////////////////////////////////////////////////////////
INLINE string PGItem::
get_exit_event() const {
  return get_exit_prefix() + get_id();
}

////////////////////////////////////////////////////////////////////
//     Function: PGItem::get_within_event
//       Access: Published
//  Description: Returns the event name that will be thrown when the
//               item is active and the mouse moves within the
//               boundaries of the frame.  This is different from the
//               enter_event in that the mouse is considered within
//               the frame even if it is also within a nested frame.
////////////////////////////////////////////////////////////////////
INLINE string PGItem::
get_within_event() const {
  return get_within_prefix() + get_id();
}

////////////////////////////////////////////////////////////////////
//     Function: PGItem::get_without_event
//       Access: Published
//  Description: Returns the event name that will be thrown when the
//               item is active and the mouse moves completely outside
//               the boundaries of the frame.  This is different from
//               the exit_event in that the mouse is considered
//               within the frame even if it is also within a nested
//               frame.
////////////////////////////////////////////////////////////////////
INLINE string PGItem::
get_without_event() const {
  return get_without_prefix() + get_id();
}

////////////////////////////////////////////////////////////////////
//     Function: PGItem::get_focus_in_event
//       Access: Published
//  Description: Returns the event name that will be thrown when the
//               item gets the keyboard focus.
////////////////////////////////////////////////////////////////////
INLINE string PGItem::
get_focus_in_event() const {
  return get_focus_in_prefix() + get_id();
}

////////////////////////////////////////////////////////////////////
//     Function: PGItem::get_focus_out_event
//       Access: Published
//  Description: Returns the event name that will be thrown when the
//               item loses the keyboard focus.
////////////////////////////////////////////////////////////////////
INLINE string PGItem::
get_focus_out_event() const {
  return get_focus_out_prefix() + get_id();
}

////////////////////////////////////////////////////////////////////
//     Function: PGItem::get_press_event
//       Access: Published
//  Description: Returns the event name that will be thrown when the
//               item is active and the indicated mouse or keyboard
//               button is depressed while the mouse is within the
//               frame.
////////////////////////////////////////////////////////////////////
INLINE string PGItem::
get_press_event(const ButtonHandle &button) const {
  return get_press_prefix() + button.get_name() + "-" + get_id();
}

////////////////////////////////////////////////////////////////////
//     Function: PGItem::get_release_event
//       Access: Published
//  Description: Returns the event name that will be thrown when the
//               item is active and the indicated mouse or keyboard
//               button, formerly clicked down is within the frame, is
//               released.
////////////////////////////////////////////////////////////////////
INLINE string PGItem::
get_release_event(const ButtonHandle &button) const {
  return get_release_prefix() + button.get_name() + "-" + get_id();
}

////////////////////////////////////////////////////////////////////
//     Function: PGItem::get_keystroke_event
//       Access: Published
//  Description: Returns the event name that will be thrown when the
//               item is active and any key is pressed by the user.
////////////////////////////////////////////////////////////////////
INLINE string PGItem::
get_keystroke_event() const {
  return get_keystroke_prefix() + get_id();
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

////////////////////////////////////////////////////////////////////
//     Function: PGItem::get_focus_item
//       Access: Published, Static
//  Description: Returns the one PGItem in the world that currently
//               has keyboard focus, if any, or NULL if no item has
//               keyboard focus.  Use PGItem::set_focus() to activate
//               or deactivate keyboard focus on a particular item.
////////////////////////////////////////////////////////////////////
INLINE PGItem *PGItem::
get_focus_item() {
  return _focus_item;
}
