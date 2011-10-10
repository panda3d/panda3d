// Filename: collisionLine.I
// Created by:  drose (05Jan05)
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
CollisionLine(const LPoint3 &origin, const LVector3 &direction) :
  CollisionRay(origin, direction)
{
}

////////////////////////////////////////////////////////////////////
//     Function: CollisionLine::Constructor
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
INLINE CollisionLine::
CollisionLine(PN_stdfloat ox, PN_stdfloat oy, PN_stdfloat oz,
              PN_stdfloat dx, PN_stdfloat dy, PN_stdfloat dz) :
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
