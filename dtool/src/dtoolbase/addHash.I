// Filename: add_hash.I
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


////////////////////////////////////////////////////////////////////
//     Function: AddHash::add_hash
//       Access: Public, Static
//  Description: Adds a linear sequence of uint32 words to the hash.
////////////////////////////////////////////////////////////////////
INLINE size_t AddHash::
add_hash(size_t start, const PN_uint32 *words, size_t num_words) {
  return (size_t)hashword(words, num_words, (PN_uint32)start);
}

////////////////////////////////////////////////////////////////////
//     Function: AddHash::add_hash
//       Access: Public, Static
//  Description: Adds a linear sequence of float32 words to the hash.
////////////////////////////////////////////////////////////////////
INLINE size_t AddHash::
add_hash(size_t start, const PN_float32 *floats, size_t num_floats) {
  return add_hash(start, (const PN_uint32 *)floats, num_floats);
}

////////////////////////////////////////////////////////////////////
//     Function: AddHash::add_hash
//       Access: Public, Static
//  Description: Adds a linear sequence of float64 words to the hash.
////////////////////////////////////////////////////////////////////
INLINE size_t AddHash::
add_hash(size_t start, const PN_float64 *floats, size_t num_floats) {
  return add_hash(start, (const PN_uint32 *)floats, num_floats * 2);
}

