// Filename: fisheyeLens.I
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
//     Function: FisheyeLens::Constructor
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
INLINE FisheyeLens::
FisheyeLens() {
}

////////////////////////////////////////////////////////////////////
//     Function: FisheyeLens::Copy Constructor
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
INLINE FisheyeLens::
FisheyeLens(const FisheyeLens &copy) : Lens(copy) {
}

////////////////////////////////////////////////////////////////////
//     Function: FisheyeLens::Copy Assignment Operator
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
INLINE void FisheyeLens::
operator = (const FisheyeLens &copy) {
  Lens::operator = (copy);
}

