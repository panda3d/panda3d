// Filename: pgButton.I
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
  return "click-" + button.get_name() + "-" + get_id();
}

////////////////////////////////////////////////////////////////////
//     Function: PGButton::is_button_down
//       Access: Published
//  Description: Returns if the value of button_down
////////////////////////////////////////////////////////////////////
INLINE bool PGButton::
is_button_down() {
  return _button_down;
}
