// Filename: pSphereLens.I
// Created by:  drose (12Dec01)
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
//     Function: PSphereLens::Constructor
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
INLINE PSphereLens::
PSphereLens() {
}

////////////////////////////////////////////////////////////////////
//     Function: PSphereLens::Copy Constructor
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
INLINE PSphereLens::
PSphereLens(const PSphereLens &copy) : Lens(copy) {
}

////////////////////////////////////////////////////////////////////
//     Function: PSphereLens::Copy Assignment Operator
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
INLINE void PSphereLens::
operator = (const PSphereLens &copy) {
  Lens::operator = (copy);
}

