// Filename: pgButtonEvent.I
// Created by:  drose (05Jul01)
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
//     Function: PGButtonEvent::Constructor
//       Access: Public
//  Description: 
////////////////////////////////////////////////////////////////////
INLINE PGButtonEvent::
PGButtonEvent() {
}

////////////////////////////////////////////////////////////////////
//     Function: PGButtonEvent::Constructor
//       Access: Public
//  Description: 
////////////////////////////////////////////////////////////////////
INLINE PGButtonEvent::
PGButtonEvent(ButtonHandle button, float mouse_x, float mouse_y) :
  _button(button),
  _mouse_x(mouse_x),
  _mouse_y(mouse_y)
{
}

////////////////////////////////////////////////////////////////////
//     Function: PGButtonEvent::get_button
//       Access: Published
//  Description: Returns the ButtonHandle of the button involved with
//               this event.
////////////////////////////////////////////////////////////////////
INLINE ButtonHandle PGButtonEvent::
get_button() const {
  return _button;
}

////////////////////////////////////////////////////////////////////
//     Function: PGButtonEvent::get_button_name
//       Access: Published
//  Description: Returns the name of the button involved with this
//               event.  This is just a shorthand way to get to the
//               name, since get_button().get_name() will return the
//               same thing.
////////////////////////////////////////////////////////////////////
INLINE string PGButtonEvent::
get_button_name() const {
  return _button.get_name();
}

////////////////////////////////////////////////////////////////////
//     Function: PGButtonEvent::get_mouse_x
//       Access: Published
//  Description: Returns the X position of the mouse within the window
//               at the time the button event occurred.  This is in
//               the normalized range [-1 .. 1].
////////////////////////////////////////////////////////////////////
INLINE float PGButtonEvent::
get_mouse_x() const {
  return _mouse_x;
}

////////////////////////////////////////////////////////////////////
//     Function: PGButtonEvent::get_mouse_y
//       Access: Published
//  Description: Returns the Y position of the mouse within the window
//               at the time the button event occurred.  This is in
//               the normalized range [-1 .. 1].
////////////////////////////////////////////////////////////////////
INLINE float PGButtonEvent::
get_mouse_y() const {
  return _mouse_y;
}

INLINE ostream &
operator << (ostream &out, const PGButtonEvent &event) {
  event.output(out);
  return out;
}
