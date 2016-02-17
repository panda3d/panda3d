/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file boxEmitter.I
 * @author charles
 * @date 2000-06-26
 */

/**
 * boundary assignment
 */
INLINE void BoxEmitter::
set_min_bound(const LPoint3& vmin) {
  _vmin = vmin;
}

/**
 * boundary assignment
 */
INLINE void BoxEmitter::
set_max_bound(const LPoint3& vmax) {
  _vmax = vmax;
}

/**
 * boundary accessor
 */
INLINE LPoint3 BoxEmitter::
get_min_bound() const {
  return _vmin;
}

/**
 * boundary accessor
 */
INLINE LPoint3 BoxEmitter::
get_max_bound() const {
  return _vmax;
}
