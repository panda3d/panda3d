// Filename: movingPartScalar.I
// Created by:  drose (23Feb99)
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
                 const float &initial_value)
  : MovingPart<ACScalarSwitchType>(parent, name, initial_value) {
}

////////////////////////////////////////////////////////////////////
//     Function: MovingPartScalar::Constructor
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
INLINE MovingPartScalar::
MovingPartScalar(void){
}
