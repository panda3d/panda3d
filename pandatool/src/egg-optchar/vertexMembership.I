// Filename: vertexMembership.I
// Created by:  drose (21Jul03)
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
