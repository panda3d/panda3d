// Filename: collisionGeom.I
// Created by:  drose (01Mar06)
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
//     Function: CollisionGeom::Constructor
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
INLINE CollisionGeom::
CollisionGeom(const LVecBase3f &a, const LVecBase3f &b,
              const LVecBase3f &c) :
  CollisionPolygon(a, b, c)
{
}

////////////////////////////////////////////////////////////////////
//     Function: CollisionGeom::Copy Constructor
//       Access: Private
//  Description:
////////////////////////////////////////////////////////////////////
CollisionGeom::
CollisionGeom(const CollisionGeom &copy) :
  CollisionPolygon(copy)
{
}
