/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file hashGeneratorBase.I
 * @author drose
 * @date 2001-05-14
 */

/**
 *
 */
INLINE HashGeneratorBase::
HashGeneratorBase() {
  _hash = 0;
}

/**
 * Returns the hash number generated.
 */
INLINE size_t HashGeneratorBase::
get_hash() const {
  return _hash;
}
