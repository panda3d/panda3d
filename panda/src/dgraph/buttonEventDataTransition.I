// Filename: buttonEventDataTransition.I
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
//     Function: ButtonEventDataTransition::Constructor
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
INLINE ButtonEventDataTransition::
ButtonEventDataTransition() {
}

////////////////////////////////////////////////////////////////////
//     Function: ButtonEventDataTransition::Copy Constructor
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
INLINE ButtonEventDataTransition::
ButtonEventDataTransition(const ButtonEventDataTransition &copy) :
  NodeTransition(copy)
{
}

////////////////////////////////////////////////////////////////////
//     Function: ButtonEventDataTransition::Copy Assignment Operator
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
INLINE void ButtonEventDataTransition::
operator = (const ButtonEventDataTransition &copy) {
  NodeTransition::operator = (copy);
}
