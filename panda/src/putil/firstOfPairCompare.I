// Filename: firstOfPairCompare.I
// Created by:  drose (04Apr00)
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
//     Function: FirstOfPairCompare::Constructor
//       Access: Public
//  Description: 
////////////////////////////////////////////////////////////////////
template<class ObjectType, class Compare>
INLINE FirstOfPairCompare<ObjectType, Compare>::
FirstOfPairCompare(Compare compare) : _compare(compare) {
}

////////////////////////////////////////////////////////////////////
//     Function: FirstOfPairCompare::operator ()
//       Access: Public
//  Description: Returns true if a sorts before b, false otherwise.
////////////////////////////////////////////////////////////////////
template<class ObjectType, class Compare>
INLINE bool FirstOfPairCompare<ObjectType, Compare>::
operator () (const ObjectType &a, const ObjectType &b) const {
  return _compare(a.first, b.first);
}
