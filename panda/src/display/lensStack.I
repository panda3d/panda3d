// Filename: lensStack.I
// Created by:  drose (25Feb02)
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
//     Function: LensStack::Constructor
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
INLINE LensStack::
LensStack() {
  _stack_level = -1;
}

////////////////////////////////////////////////////////////////////
//     Function: LensStack::Destructor
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
INLINE LensStack::
~LensStack() {
}

////////////////////////////////////////////////////////////////////
//     Function: LensStack::Copy Constructor
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
INLINE LensStack::
LensStack(const LensStack &copy) :
  _lens(copy._lens),
  _stack_level(copy._stack_level)
{
}

////////////////////////////////////////////////////////////////////
//     Function: LensStack::Copy Operator
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
INLINE void LensStack::
operator =(const LensStack &copy) {
  _lens = copy._lens;
  _stack_level = copy._stack_level;
}
