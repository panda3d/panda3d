// Filename: linesmoothTransition.I
// Created by:  drose (24Mar00)
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
//     Function: LinesmoothTransition::Constructor
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
INLINE LinesmoothTransition::
LinesmoothTransition() {
  set_on();
}

////////////////////////////////////////////////////////////////////
//     Function: LinesmoothTransition::off
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
INLINE LinesmoothTransition LinesmoothTransition::
off() {
  LinesmoothTransition result;
  result.set_off();
  return result;
}
