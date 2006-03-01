// Filename: collisionGeom.I
// Created by:  drose (01Mar06)
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
