// Filename: collisionLine.I
// Created by:  drose (05Jan05)
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
//     Function: CollisionLine::Default Constructor
//       Access: Public
//  Description: Creates an invalid line.  This isn't terribly useful;
//               it's expected that the user will subsequently adjust
//               the line via set_origin()/set_direction() or
//               set_from_lens().
////////////////////////////////////////////////////////////////////
INLINE CollisionLine::
CollisionLine() {
}

////////////////////////////////////////////////////////////////////
//     Function: CollisionLine::Constructor
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
INLINE CollisionLine::
CollisionLine(const LPoint3f &origin, const LVector3f &direction) :
  CollisionRay(origin, direction)
{
}

////////////////////////////////////////////////////////////////////
//     Function: CollisionLine::Constructor
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
INLINE CollisionLine::
CollisionLine(float ox, float oy, float oz,
              float dx, float dy, float dz) :
  CollisionRay(ox, oy, oz, dx, dy, dz)
{
}

////////////////////////////////////////////////////////////////////
//     Function: CollisionLine::Copy Constructor
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
INLINE CollisionLine::
CollisionLine(const CollisionLine &copy) :
  CollisionRay(copy)
{
}
