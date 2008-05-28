// Filename: firstOfPairLess.I
// Created by:  drose (04Apr00)
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
//     Function: FirstOfPairLess::operator ()
//       Access: Public
//  Description: Returns true if a sorts before b, false otherwise.
////////////////////////////////////////////////////////////////////
template<class ObjectType>
INLINE bool FirstOfPairLess<ObjectType>::
operator () (const ObjectType &a, const ObjectType &b) const {
  return a.first < b.first;
}
