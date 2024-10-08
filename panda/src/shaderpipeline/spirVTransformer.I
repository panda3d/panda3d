/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file spirVTransformer.I
 * @author rdb
 * @date 2024-10-08
 */

/**
 * Returns the number of ids allocated.
 */
INLINE uint32_t SpirVTransformer::
get_id_bound() const {
  return _preamble[3];
}

/**
 *
 */
INLINE const SpirVResultDatabase &SpirVTransformer::
get_db() const {
  return _db;
}
