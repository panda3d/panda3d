// Filename: buttonEventDataAttribute.I
// Created by:  drose (27Mar00)
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
//     Function: ButtonEventDataAttribute::Constructor
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
INLINE ButtonEventDataAttribute::
ButtonEventDataAttribute() {
}

////////////////////////////////////////////////////////////////////
//     Function: ButtonEventDataAttribute::Copy Constructor
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
INLINE ButtonEventDataAttribute::
ButtonEventDataAttribute(const ButtonEventDataAttribute &copy) :
  NodeAttribute(copy),
  _buttons(copy._buttons)
{
}

////////////////////////////////////////////////////////////////////
//     Function: ButtonEventDataAttribute::Copy Assignment Operator
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
INLINE void ButtonEventDataAttribute::
operator = (const ButtonEventDataAttribute &copy) {
  NodeAttribute::operator = (copy);
  _buttons = copy._buttons;
}

////////////////////////////////////////////////////////////////////
//     Function: ButtonEventDataAttribute::begin
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
INLINE ButtonEventDataAttribute::const_iterator ButtonEventDataAttribute::
begin() const {
  return _buttons.begin();
}

////////////////////////////////////////////////////////////////////
//     Function: ButtonEventDataAttribute::end
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
INLINE ButtonEventDataAttribute::const_iterator ButtonEventDataAttribute::
end() const {
  return _buttons.end();
}

////////////////////////////////////////////////////////////////////
//     Function: ButtonEventDataAttribute::clear
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
INLINE void ButtonEventDataAttribute::
clear() {
  _buttons.clear();
}

////////////////////////////////////////////////////////////////////
//     Function: ButtonEventDataAttribute::clear
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
INLINE void ButtonEventDataAttribute::
push_back(const ButtonEvent &event) {
  _buttons.push_back(event);
}
