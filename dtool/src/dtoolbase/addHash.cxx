/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file addHash.cxx
 * @author drose
 * @date 2006-09-01
 */

#include "addHash.h"

/**
 * Adds a linear sequence of bytes to the hash.
 */
size_t AddHash::
add_hash(size_t start, const uint8_t *bytes, size_t num_bytes) {
  size_t num_words = num_bytes >> 2;
  size_t remaining_bytes = num_bytes - (num_words << 2);
  size_t hash = (size_t)hashword((const uint32_t *)bytes, num_words, (uint32_t)start);

  switch (remaining_bytes) {
  case 3:
    {
      uint32_t remaining;
      remaining = (bytes[num_bytes - 3] << 16) | (bytes[num_bytes - 2] << 8) | (bytes[num_bytes - 1]);
      hash = (size_t)hashword(&remaining, 1, (uint32_t)hash);
    }
    break;

  case 2:
    {
      uint32_t remaining;
      remaining = (bytes[num_bytes - 2] << 8) | (bytes[num_bytes - 1]);
      hash = (size_t)hashword(&remaining, 1, (uint32_t)hash);
    }
    break;

  case 1:
    {
      uint32_t remaining;
      remaining = (bytes[num_bytes - 1]);
      hash = (size_t)hashword(&remaining, 1, (uint32_t)hash);
    }
    break;

  default:
    break;
  }
  return hash;
}
