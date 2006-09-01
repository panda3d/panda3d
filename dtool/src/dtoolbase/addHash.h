// Filename: addHash.h
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

#ifndef ADDHASH_H
#define ADDHASH_H

#include "dtoolbase.h"
#include "numeric_types.h"
#include "lookup3.h"

////////////////////////////////////////////////////////////////////
//       Class : AddHash
// Description : This class is used just as a namespace scope to
//               collect together a handful of static functions, which
//               are used to wrap calls to Bob Jenkins' public-domain
//               hash generation function (defined in lookup3.c).
////////////////////////////////////////////////////////////////////
class EXPCL_DTOOL AddHash {
public:
  INLINE static size_t add_hash(size_t start, const PN_uint32 *words, size_t num_words);
  static size_t add_hash(size_t start, const PN_uint8 *bytes, size_t num_bytes);
  INLINE static size_t add_hash(size_t start, const PN_float32 *floats, size_t num_floats);
  INLINE static size_t add_hash(size_t start, const PN_float64 *floats, size_t num_floats);
};

#include "addHash.I"

#endif
