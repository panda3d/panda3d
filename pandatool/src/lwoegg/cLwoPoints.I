// Filename: cLwoPoints.I
// Created by:  drose (25Apr01)
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
//     Function: CLwoPoints::Constructor
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
INLINE CLwoPoints::
CLwoPoints(LwoToEggConverter *converter, const LwoPoints *points,
           CLwoLayer *layer) :
  _converter(converter),
  _points(points),
  _layer(layer)
{
}
