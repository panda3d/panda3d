// Filename: indirectCompareTo.I
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
//     Function: IndirectCompareTo::operator ()
//       Access: Public
//  Description: Returns true if a sorts before b, false otherwise.
////////////////////////////////////////////////////////////////////
template<class ObjectType>
INLINE bool IndirectCompareTo<ObjectType>::
operator () (const ObjectType *a, const ObjectType *b) const {
  return (a != b && a->compare_to(*b) < 0);
}
