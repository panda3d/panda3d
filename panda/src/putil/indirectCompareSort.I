// Filename: indirectCompareSort.I
// Created by:  drose (01Mar05)
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
//     Function: IndirectCompareSort::operator ()
//       Access: Public
//  Description: Returns true if a sorts before b, false otherwise.
////////////////////////////////////////////////////////////////////
template<class ObjectType>
INLINE bool IndirectCompareSort<ObjectType>::
operator () (const ObjectType *a, const ObjectType *b) const {
  return (a->get_sort() < b->get_sort());
}
