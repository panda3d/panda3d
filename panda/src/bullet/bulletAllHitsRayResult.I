// Filename: bulletAllHitsRayResult.I
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
//     Function: BulletAllHitsRayResult::empty
//       Access: Published
//  Description: Named constructor intended to be used for asserts
//               with have to return a concrete value.
////////////////////////////////////////////////////////////////////
INLINE BulletAllHitsRayResult BulletAllHitsRayResult::
empty() {

  btVector3 from;
  btVector3 to;

  return BulletAllHitsRayResult(from, to, CollideMask::all_on());
}

////////////////////////////////////////////////////////////////////
//     Function: BulletRayHit::empty
//       Access: Published
//  Description: Named constructor intended to be used for asserts
//               with have to return a concrete value.
////////////////////////////////////////////////////////////////////
INLINE BulletRayHit BulletRayHit::
empty() {

  return BulletRayHit();
}

