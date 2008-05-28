// Filename: addHash.cxx
// Created by:  drose (01Sep06)
//
////////////////////////////////////////////////////////////////////
//
// PANDA 3D SOFTWARE
// Copyright (c) Carnegie Mellon University.  All rights reserved.
//
// All use of this software is subject to the terms of the revised BSD
// license.  You should have received a copy of this license along
// with this source code in a file named "LICENSE."
//
////////////////////////////////////////////////////////////////////

#include "addHash.h"

////////////////////////////////////////////////////////////////////
//     Function: AddHash::add_hash
//       Access: Public, Static
//  Description: Adds a linear sequence of bytes to the hash.
////////////////////////////////////////////////////////////////////
size_t AddHash::
add_hash(size_t start, const PN_uint8 *bytes, size_t num_bytes) {
  size_t num_words = num_bytes >> 2;
  size_t remaining_bytes = num_bytes - (num_words << 2);
  size_t hash = (size_t)hashword((const PN_uint32 *)bytes, num_words, (PN_uint32)start);

  switch (remaining_bytes) {
  case 3:
    {
      PN_uint32 remaining;
      remaining = (bytes[num_bytes - 3] << 16) | (bytes[num_bytes - 2] << 8) | (bytes[num_bytes - 1]);
      hash = (size_t)hashword(&remaining, 1, (PN_uint32)hash);
    }
    break;

  case 2:
    {
      PN_uint32 remaining;
      remaining = (bytes[num_bytes - 2] << 8) | (bytes[num_bytes - 1]);
      hash = (size_t)hashword(&remaining, 1, (PN_uint32)hash);
    }
    break;

  case 1:
    {
      PN_uint32 remaining;
      remaining = (bytes[num_bytes - 1]);
      hash = (size_t)hashword(&remaining, 1, (PN_uint32)hash);
    }
    break;

  default:
    break;
  }
  return hash;
}
