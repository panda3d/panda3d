// Filename: add_hash.I
// Created by:  drose (01Sep06)
//
////////////////////////////////////////////////////////////////////
//
// PANDA 3D SOFTWARE
// Copyright (c) 2001 - 2004, Disney Enterprises, Inc.  All rights reserved
//
// All use of this software is subject to the terms of the Panda 3d
// Software license.  You should have received a copy of this license
// along with this source code; you will also find a current copy of
// the license at http://etc.cmu.edu/panda3d/docs/license/ .
//
// To contact the maintainers of this program write to
// panda3d-general@lists.sourceforge.net .
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

