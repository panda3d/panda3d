// Filename: pgButton.I
// Created by:  drose (03Jul01)
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
//     Function: PGButton::setup
//       Access: Published
//  Description: Sets up the button using the indicated NodePath as
//               arbitrary geometry.
////////////////////////////////////////////////////////////////////
INLINE void PGButton::
setup(const ArcChain &ready) {
  setup(ready, ready, ready, ready);
}

////////////////////////////////////////////////////////////////////
//     Function: PGButton::setup
//       Access: Published
//  Description: Sets up the button using the indicated NodePath as
//               arbitrary geometry.
////////////////////////////////////////////////////////////////////
INLINE void PGButton::
setup(const ArcChain &ready, const ArcChain &depressed) {
  setup(ready, depressed, ready, ready);
}

////////////////////////////////////////////////////////////////////
//     Function: PGButton::setup
//       Access: Published
//  Description: Sets up the button using the indicated NodePath as
//               arbitrary geometry.
////////////////////////////////////////////////////////////////////
INLINE void PGButton::
setup(const ArcChain &ready, const ArcChain &depressed, 
      const ArcChain &rollover) {
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
