/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file checksumHashGenerator.I
 * @author drose
 * @date 2001-05-14
 */

#ifdef _WIN32
// Needed for PtrToLong, below
#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN 1
#endif
#include <windows.h>
#endif

/**
 * Adds another integer to the hash so far.  This function should be
 * overridden in base classes; this is the principle implementation of the
 * HashGenerator.
 */
INLINE void ChecksumHashGenerator::
add_int(long sum) {
  _hash += (size_t)sum;
}

/**
 * Adds a boolean flag.
 */
INLINE void ChecksumHashGenerator::
add_bool(bool flag) {
  add_int(flag);
}

/**
 * Adds a floating-point number, first converting it to fixed point by
 * dividing it by the indicated threshold.
 */
INLINE void ChecksumHashGenerator::
add_fp(float number, float threshold) {
  add_int((long)(number / threshold));
}

/**
 * Adds a floating-point number, first converting it to fixed point by
 * dividing it by the indicated threshold.
 */
INLINE void ChecksumHashGenerator::
add_fp(double number, double threshold) {
  add_int((long)(number / threshold));
}

/**
 * Adds a pointer, derived simply by casting the pointer to an integer.  This
 * should be good enough even on architectures for which this cast is lossy.
 */
INLINE void ChecksumHashGenerator::
add_pointer(void *ptr) {
#ifdef _WIN32
  add_int(PtrToLong(ptr));
#else
  add_int((long)ptr);
#endif
}
