/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file bitArray_ext.I
 * @author rdb
 * @date 2020-03-21
 */

/**
 * Returns true if the value is not zero.
 */
INLINE bool Extension<BitArray>::
__bool__() const {
  return !_this->is_zero();
}
