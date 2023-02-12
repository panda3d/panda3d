/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file bulletAllHitsRayResult.I
 * @author enn0x
 * @date 2010-02-21
 */

/**
 * Named constructor intended to be used for asserts with have to return a
 * concrete value.
 */
INLINE BulletAllHitsRayResult BulletAllHitsRayResult::
empty() {

  return BulletAllHitsRayResult(btVector3(0, 0, 0), btVector3(0, 0, 0), CollideMask::all_on());
}

/**
 * Named constructor intended to be used for asserts with have to return a
 * concrete value.
 */
INLINE BulletRayHit BulletRayHit::
empty() {

  return BulletRayHit();
}
