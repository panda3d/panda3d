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
//     Function: PGButton::set_active
//       Access: Public
//  Description: Toggles the active/inactive state of the button.  In
//               the case of a PGButton, this also changes its visual
//               appearance.
////////////////////////////////////////////////////////////////////
INLINE void PGButton:: 
set_active(bool active) {
  if (active != get_active()) {
    set_state(active ? S_ready : S_inactive);
    PGItem::set_active(active);
  }
}

////////////////////////////////////////////////////////////////////
//     Function: PGButton::get_click_event
//       Access: Published
//  Description: Returns the event name that will be thrown when the
//               button is clicked normally.
////////////////////////////////////////////////////////////////////
INLINE string PGButton::
get_click_event() const {
  return "click-" + get_id();
}
