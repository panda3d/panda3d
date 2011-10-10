// Filename: collisionInvSphere.I
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
//     Function: CollisionInvSphere::Constructor
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
INLINE CollisionInvSphere::
CollisionInvSphere(const LPoint3 &center, PN_stdfloat radius) :
  CollisionSphere(center, radius)
{
}

////////////////////////////////////////////////////////////////////
//     Function: CollisionInvSphere::Constructor
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
INLINE CollisionInvSphere::
CollisionInvSphere(PN_stdfloat cx, PN_stdfloat cy, PN_stdfloat cz, PN_stdfloat radius) :
  CollisionSphere(cx, cy, cz, radius)
{
}

////////////////////////////////////////////////////////////////////
//     Function: CollisionInvSphere::Default constructor
//       Access: Protected
//  Description: Creates an invalid sphere.  Only used when reading
//               from a bam file.
////////////////////////////////////////////////////////////////////
INLINE CollisionInvSphere::
CollisionInvSphere() {
}

////////////////////////////////////////////////////////////////////
//     Function: CollisionInvSphere::Copy Constructor
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
INLINE CollisionInvSphere::
CollisionInvSphere(const CollisionInvSphere &copy) :
  CollisionSphere(copy)
{
}
