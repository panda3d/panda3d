// Filename: vertexMembership.I
// Created by:  drose (21Jul03)
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
//     Function: VertexMembership::Constructor
//       Access: Public
//  Description: 
////////////////////////////////////////////////////////////////////
INLINE VertexMembership::
VertexMembership(EggGroup *group, double membership) :
  _group(group),
  _membership(membership)
{
}

////////////////////////////////////////////////////////////////////
//     Function: VertexMembership::Copy Constructor
//       Access: Public
//  Description: 
////////////////////////////////////////////////////////////////////
INLINE VertexMembership::
VertexMembership(const VertexMembership &copy) :
  _group(copy._group),
  _membership(copy._membership)
{
}

////////////////////////////////////////////////////////////////////
//     Function: VertexMembership::Copy Assignment Operator
//       Access: Public
//  Description: 
////////////////////////////////////////////////////////////////////
INLINE void VertexMembership::
operator = (const VertexMembership &copy) {
  _group = copy._group;
  _membership = copy._membership;
}

////////////////////////////////////////////////////////////////////
//     Function: VertexMembership::Ordering Operator
//       Access: Public
//  Description: 
////////////////////////////////////////////////////////////////////
INLINE bool VertexMembership::
operator < (const VertexMembership &other) const {
  if (_membership != other._membership) {
    return _membership < other._membership;
  }
  return _group < other._group;
}
