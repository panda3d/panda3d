// Filename: immediateTransition.I
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
//     Function: ImmediateTransition::Constructor
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
INLINE_GRAPH ImmediateTransition::
ImmediateTransition() {
}

////////////////////////////////////////////////////////////////////
//     Function: ImmediateTransition::Copy Constructor
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
INLINE_GRAPH ImmediateTransition::
ImmediateTransition(const ImmediateTransition &copy) :
  NodeTransition(copy)
{
}

////////////////////////////////////////////////////////////////////
//     Function: ImmediateTransition::Copy Assignment Operator
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
INLINE_GRAPH void ImmediateTransition::
operator = (const ImmediateTransition &copy) {
  NodeTransition::operator = (copy);
}
