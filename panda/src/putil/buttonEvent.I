// Filename: buttonEvent.I
// Created by:  drose (01Mar00)
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
//     Function: ButtonEvent::Default Constructor
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
INLINE ButtonEvent::
ButtonEvent() :
  _button(ButtonHandle::none()),
  _down(false)
{
}

////////////////////////////////////////////////////////////////////
//     Function: ButtonEvent::Constructor
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
INLINE ButtonEvent::
ButtonEvent(ButtonHandle button, bool down) :
  _button(button),
  _down(down)
{
}

////////////////////////////////////////////////////////////////////
//     Function: ButtonEvent::Copy Constructor
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
INLINE ButtonEvent::
ButtonEvent(const ButtonEvent &copy) :
  _button(copy._button),
  _down(copy._down)
{
}

////////////////////////////////////////////////////////////////////
//     Function: ButtonEvent::Copy Assignment Operator
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
INLINE void ButtonEvent::
operator = (const ButtonEvent &copy) {
  _button = copy._button;
  _down = copy._down;
}

////////////////////////////////////////////////////////////////////
//     Function: ButtonEvent::Equality Operator
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
INLINE bool ButtonEvent::
operator == (const ButtonEvent &other) const {
  return (_button == other._button &&
          _down == other._down);
}

////////////////////////////////////////////////////////////////////
//     Function: ButtonEvent::Inequality Operator
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
INLINE bool ButtonEvent::
operator != (const ButtonEvent &other) const {
  return !operator == (other);
}

////////////////////////////////////////////////////////////////////
//     Function: ButtonEvent::Ordering Operator
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
INLINE bool ButtonEvent::
operator < (const ButtonEvent &other) const {
  if (_button != other._button) {
    return _button < other._button;
  }

  return _down < other._down;
}
