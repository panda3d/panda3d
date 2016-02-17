/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file tangentRingEmitter.I
 * @author charles
 * @date 2000-07-25
 */

/**

 */
INLINE void TangentRingEmitter::
set_radius(PN_stdfloat r) {
  _radius = r;
}

/**

 */
INLINE void TangentRingEmitter::
set_radius_spread(PN_stdfloat spread) {
  _radius_spread = spread;
}

/**

 */
INLINE PN_stdfloat TangentRingEmitter::
get_radius() const {
  return _radius;
}

/**

 */
INLINE PN_stdfloat TangentRingEmitter::
get_radius_spread() const {
  return _radius_spread;
}
