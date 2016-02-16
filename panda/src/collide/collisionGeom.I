/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file collisionGeom.I
 * @author drose
 * @date 2006-03-01
 */

////////////////////////////////////////////////////////////////////
//     Function: CollisionGeom::Constructor
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
INLINE CollisionGeom::
CollisionGeom(const LVecBase3 &a, const LVecBase3 &b,
              const LVecBase3 &c) :
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
