// Filename: movingPartScalar.I
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
//     Function: MovingPartScalar::Copy Constructor
//       Access: Protected
//  Description: Normally, you'd use make_copy() or copy_subgraph() to
//               make a copy of this.
////////////////////////////////////////////////////////////////////
INLINE MovingPartScalar::
MovingPartScalar(const MovingPartScalar &copy) :
  MovingPart<ACScalarSwitchType>(copy)
{
}

////////////////////////////////////////////////////////////////////
//     Function: MovingPartScalar::Constructor
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
INLINE MovingPartScalar::
MovingPartScalar(PartGroup *parent, const string &name,
                 const PN_stdfloat &default_value)
  : MovingPart<ACScalarSwitchType>(parent, name, default_value) {
}

////////////////////////////////////////////////////////////////////
//     Function: MovingPartScalar::Constructor
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
INLINE MovingPartScalar::
MovingPartScalar(){
}
