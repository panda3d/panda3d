/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file nativeNumericData.h
 * @author drose
 * @date 2001-05-09
 */

#ifndef NATIVENUMERICDATA_H
#define NATIVENUMERICDATA_H

#include "dtoolbase.h"
#include "numeric_types.h"

#include <string.h>  // for memcpy()

/**
 * NativeNumericData and ReversedNumericData work together to provide a sneaky
 * interface for automatically byte-swapping numbers, when necessary, to
 * transparency support big-endian and little-endian architectures.
 *
 * Both of these classes provide interfaces that accept a pointer to a numeric
 * variable and the size of the number, and they can append that data to the
 * end of a string, or memcpy it into another location.
 *
 * The difference is that NativeNumericData simply passes everything through
 * unchanged, while ReversedNumericData always byte-swaps everything.
 * Otherwise, they have the same interface.
 *
 * The transparent part comes from LittleEndian and BigEndian, which are
 * typedeffed to be one of these or the other, according to the machine's
 * architecture.
 */
class EXPCL_DTOOL_PRC NativeNumericData {
public:
  INLINE NativeNumericData(const void *data, size_t length);
  INLINE NativeNumericData(const void *data, size_t start, size_t length);

  INLINE void store_value(void *dest, size_t length) const;
  INLINE const void *get_data() const;

private:
  const void *_source;
};

#include "nativeNumericData.I"

#endif
