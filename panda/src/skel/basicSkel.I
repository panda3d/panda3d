/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file basicSkel.I
 * @author jyelon
 * @date 2007-01-31
 */

/**
 *
 */
INLINE BasicSkel::
BasicSkel() {
}

/**
 *
 */
INLINE BasicSkel::
~BasicSkel() {
}

/**
 * Stores an integer value.
 */
INLINE void BasicSkel::
set_value(int n) {
  _value = n;
}

/**
 * Retreives a value that was previously stored.
 */
INLINE int BasicSkel::
get_value() const {
  return _value;
}
