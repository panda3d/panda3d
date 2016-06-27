/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file lightRampAttrib.I
 * @author drose
 * @date 2002-03-04
 */

/**
 * Use LightRampAttrib::make() to construct a new LightRampAttrib object.
 */
INLINE LightRampAttrib::
LightRampAttrib() {
  _mode = LRT_default;
  _level[0] = 0.0;
  _level[1] = 0.0;
  _threshold[0] = 0.0;
  _threshold[1] = 0.0;
}

/**
 * Returns the LightRampAttrib mode.
 */
INLINE LightRampAttrib::LightRampMode LightRampAttrib::
get_mode() const {
  return _mode;
}

/**
 * Returns the nth lighting level.
 */
INLINE PN_stdfloat LightRampAttrib::
get_level(int n) const {
  if ((n<0)||(n>1)) return 0.0;
  return _level[n];
}

/**
 * Returns the nth threshold level.
 */
INLINE PN_stdfloat LightRampAttrib::
get_threshold(int n) const {
  if ((n<0)||(n>1)) return 0.0;
  return _threshold[n];
}
