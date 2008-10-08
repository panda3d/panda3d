// Filename: pgButton.I
// Created by:  drose (13Mar02)
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
//     Function: PGButton::set_notify
//       Access: Published
//  Description: Sets the object which will be notified when the
//               PGButton changes.  Set this to NULL to disable
//               this effect.  The PGButton does not retain
//               ownership of the pointer; it is your responsibility
//               to ensure that the notify object does not destruct.
////////////////////////////////////////////////////////////////////
INLINE void PGButton:: 
set_notify(PGButtonNotify *notify) {
  PGItem::set_notify(notify);
}

////////////////////////////////////////////////////////////////////
//     Function: PGButton::get_notify
//       Access: Published
//  Description: Returns the object which will be notified when the
//               PGButton changes, if any.  Returns NULL if there
//               is no such object configured.
////////////////////////////////////////////////////////////////////
INLINE PGButtonNotify *PGButton:: 
get_notify() const {
  return (PGButtonNotify *)PGItem::get_notify();
}

////////////////////////////////////////////////////////////////////
//     Function: PGButton::setup
//       Access: Published
//  Description: Sets up the button using the indicated NodePath as
//               arbitrary geometry.
////////////////////////////////////////////////////////////////////
INLINE void PGButton::
setup(const NodePath &ready) {
  setup(ready, ready, ready, ready);
}

////////////////////////////////////////////////////////////////////
//     Function: PGButton::setup
//       Access: Published
//  Description: Sets up the button using the indicated NodePath as
//               arbitrary geometry.
////////////////////////////////////////////////////////////////////
INLINE void PGButton::
setup(const NodePath &ready, const NodePath &depressed) {
  setup(ready, depressed, ready, ready);
}

////////////////////////////////////////////////////////////////////
//     Function: PGButton::setup
//       Access: Published
//  Description: Sets up the button using the indicated NodePath as
//               arbitrary geometry.
////////////////////////////////////////////////////////////////////
INLINE void PGButton::
setup(const NodePath &ready, const NodePath &depressed, 
      const NodePath &rollover) {
  setup(ready, depressed, rollover, ready);
}

////////////////////////////////////////////////////////////////////
//     Function: PGButton::get_click_prefix
//       Access: Published, Static
//  Description: Returns the prefix that is used to define the click
//               event for all PGButtons.  The click event is the
//               concatenation of this string followed by get_id().
////////////////////////////////////////////////////////////////////
INLINE string PGButton::
get_click_prefix() {
  return "click-";
}

////////////////////////////////////////////////////////////////////
//     Function: PGButton::get_click_event
//       Access: Published
//  Description: Returns the event name that will be thrown when the
//               button is clicked normally.
////////////////////////////////////////////////////////////////////
INLINE string PGButton::
get_click_event(const ButtonHandle &button) const {
  LightReMutexHolder holder(_lock);
  return get_click_prefix() + button.get_name() + "-" + get_id();
}

////////////////////////////////////////////////////////////////////
//     Function: PGButton::is_button_down
//       Access: Published
//  Description: Returns true if the user is currently holding the
//               mouse button down on the button, false otherwise.
////////////////////////////////////////////////////////////////////
INLINE bool PGButton::
is_button_down() {
  LightReMutexHolder holder(_lock);
  return _button_down;
}
