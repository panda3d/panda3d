// Filename: collisionInvSphere.I
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
//     Function: CollisionInvSphere::Constructor
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
INLINE CollisionInvSphere::
CollisionInvSphere(const LPoint3f &center, float radius) :
  CollisionSphere(center, radius)
{
}

////////////////////////////////////////////////////////////////////
//     Function: CollisionInvSphere::Constructor
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
INLINE CollisionInvSphere::
CollisionInvSphere(float cx, float cy, float cz, float radius) :
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
