/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file lineEmitter.I
 * @author charles
 * @date 2000-06-26
 */

/**
 * endpoint assignment
 */
INLINE void LineEmitter::
set_endpoint1(const LPoint3& point) {
  _endpoint1 = point;
}

/**
 * endpoint assignment
 */
INLINE void LineEmitter::
set_endpoint2(const LPoint3& point) {
  _endpoint2 = point;
}

/**
 * endpoint accessor
 */
INLINE LPoint3 LineEmitter::
get_endpoint1() const {
  return _endpoint1;
}

/**
 * endpoint accessor
 */
INLINE LPoint3 LineEmitter::
get_endpoint2() const {
  return _endpoint2;
}
