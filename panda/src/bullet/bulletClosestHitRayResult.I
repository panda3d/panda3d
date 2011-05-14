// Filename: bulletClosestHitRayResult.I
// Created by:  enn0x (21Feb10)
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
//     Function: BulletClosestHitRayResult::empty
//       Access: Public
//  Description: Named constructor intended to be used for asserts
//               with have to return a concrete value.
////////////////////////////////////////////////////////////////////
INLINE BulletClosestHitRayResult BulletClosestHitRayResult::
empty() {

  btVector3 from;
  btVector3 to;

  return BulletClosestHitRayResult(from, to, CollideMask::all_on());
}

