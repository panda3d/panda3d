/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file addHash.I
 * @author drose
 * @date 2006-09-01
 */

/**
 * Adds a linear sequence of uint32 words to the hash.
 */
INLINE size_t AddHash::
add_hash(size_t start, const uint32_t *words, size_t num_words) {
  return (size_t)hashword(words, num_words, (uint32_t)start);
}

/**
 * Adds a linear sequence of float32 words to the hash.
 */
INLINE size_t AddHash::
add_hash(size_t start, const PN_float32 *floats, size_t num_floats) {
  return add_hash(start, (const uint32_t *)floats, num_floats);
}

/**
 * Adds a linear sequence of float64 words to the hash.
 */
INLINE size_t AddHash::
add_hash(size_t start, const PN_float64 *floats, size_t num_floats) {
  return add_hash(start, (const uint32_t *)floats, num_floats * 2);
}
