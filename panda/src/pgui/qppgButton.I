// Filename: qppgButton.I
// Created by:  drose (13Mar02)
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
//     Function: qpPGButton::setup
//       Access: Published
//  Description: Sets up the button using the indicated NodePath as
//               arbitrary geometry.
////////////////////////////////////////////////////////////////////
INLINE void qpPGButton::
setup(const qpNodePath &ready) {
  setup(ready, ready, ready, ready);
}

////////////////////////////////////////////////////////////////////
//     Function: qpPGButton::setup
//       Access: Published
//  Description: Sets up the button using the indicated NodePath as
//               arbitrary geometry.
////////////////////////////////////////////////////////////////////
INLINE void qpPGButton::
setup(const qpNodePath &ready, const qpNodePath &depressed) {
  setup(ready, depressed, ready, ready);
}

////////////////////////////////////////////////////////////////////
//     Function: qpPGButton::setup
//       Access: Published
//  Description: Sets up the button using the indicated NodePath as
//               arbitrary geometry.
////////////////////////////////////////////////////////////////////
INLINE void qpPGButton::
setup(const qpNodePath &ready, const qpNodePath &depressed, 
      const qpNodePath &rollover) {
  setup(ready, depressed, rollover, ready);
}

////////////////////////////////////////////////////////////////////
//     Function: qpPGButton::get_click_prefix
//       Access: Published, Static
//  Description: Returns the prefix that is used to define the click
//               event for all qpPGButtons.  The click event is the
//               concatenation of this string followed by get_id().
////////////////////////////////////////////////////////////////////
INLINE string qpPGButton::
get_click_prefix() {
  return "click-";
}

////////////////////////////////////////////////////////////////////
//     Function: qpPGButton::get_click_event
//       Access: Published
//  Description: Returns the event name that will be thrown when the
//               button is clicked normally.
////////////////////////////////////////////////////////////////////
INLINE string qpPGButton::
get_click_event(const ButtonHandle &button) const {
  return "click-" + button.get_name() + "-" + get_id();
}
