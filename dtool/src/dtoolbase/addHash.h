/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file addHash.h
 * @author drose
 * @date 2006-09-01
 */

#ifndef ADDHASH_H
#define ADDHASH_H

#include "dtoolbase.h"
#include "numeric_types.h"
#include "lookup3.h"

/**
 * This class is used just as a namespace scope to collect together a handful
 * of static functions, which are used to wrap calls to Bob Jenkins' public-
 * domain hash generation function (defined in lookup3.c).
 */
class EXPCL_DTOOL_DTOOLBASE AddHash {
public:
  INLINE static size_t add_hash(size_t start, const uint32_t *words, size_t num_words);
  static size_t add_hash(size_t start, const uint8_t *bytes, size_t num_bytes);
  INLINE static size_t add_hash(size_t start, const PN_float32 *floats, size_t num_floats);
  INLINE static size_t add_hash(size_t start, const PN_float64 *floats, size_t num_floats);
};

#include "addHash.I"

#endif
