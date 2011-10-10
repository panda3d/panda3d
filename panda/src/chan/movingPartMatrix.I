// Filename: movingPartMatrix.I
// Created by:  drose (23Feb99)
//
////////////////////////////////////////////////////////////////////
//
// PANDA 3D SOFTWARE
// Copyright (c) Carnegie Mellon University.  All rights reserved.
//
// All use of this software is subject to the terms of the revised BSD
// license.  You should have received a copy of this license along
// with this source code in a file named "LICENSE."
//
////////////////////////////////////////////////////////////////////


////////////////////////////////////////////////////////////////////
//     Function: MovingPartMatrix::Copy Constructor
//       Access: Protected
//  Description: Normally, you'd use make_copy() or copy_subgraph() to
//               make a copy of this.
////////////////////////////////////////////////////////////////////
INLINE MovingPartMatrix::
MovingPartMatrix(const MovingPartMatrix &copy) :
  MovingPart<ACMatrixSwitchType>(copy)
{
}

////////////////////////////////////////////////////////////////////
//     Function: MovingPartMatrix::Constructor
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
INLINE MovingPartMatrix::
MovingPartMatrix(PartGroup *parent, const string &name,
                 const LMatrix4 &default_value)
  : MovingPart<ACMatrixSwitchType>(parent, name, default_value) {
}

////////////////////////////////////////////////////////////////////
//     Function: MovingPartMatrix::Constructor
//       Access: Protected
//  Description:
////////////////////////////////////////////////////////////////////
INLINE MovingPartMatrix::
MovingPartMatrix() {
}
