/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file rectangleEmitter.I
 * @author charles
 * @date 2000-06-26
 */

/**
 * boundary set
 */
INLINE void RectangleEmitter::
set_min_bound(const LPoint2& vmin) {
  _vmin = vmin;
}

/**
 * boundary set
 */
INLINE void RectangleEmitter::
set_max_bound(const LPoint2& vmax) {
  _vmax = vmax;
}

/**
 * boundary get
 */
INLINE LPoint2 RectangleEmitter::
get_min_bound() const {
  return _vmin;
}

/**
 * boundary get
 */
INLINE LPoint2 RectangleEmitter::
get_max_bound() const {
  return _vmax;
}
