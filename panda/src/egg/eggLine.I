// Filename: eggLine.I
// Created by:  drose (14Oct03)
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
//     Function: EggLine::Constructor
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
INLINE EggLine::
EggLine(const string &name) : EggPrimitive(name) {
}

////////////////////////////////////////////////////////////////////
//     Function: EggLine::Copy constructor
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
INLINE EggLine::
EggLine(const EggLine &copy) : EggPrimitive(copy) {
}

////////////////////////////////////////////////////////////////////
//     Function: EggLine::Copy assignment operator
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
INLINE EggLine &EggLine::
operator = (const EggLine &copy) {
  EggPrimitive::operator = (copy);
  return *this;
}
