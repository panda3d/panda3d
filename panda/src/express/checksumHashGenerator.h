/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file checksumHashGenerator.h
 * @author drose
 * @date 2001-05-14
 */

#ifndef CHECKSUMHASHGENERATOR_H
#define CHECKSUMHASHGENERATOR_H

#include "pandabase.h"

#include "hashGeneratorBase.h"

/**
 * This is a specific kind of HashGenerator that simply adds up all of the
 * ints.  Nothing fancy, and pretty quick.
 */
class EXPCL_PANDA_EXPRESS ChecksumHashGenerator : public HashGeneratorBase {
public:
  INLINE void add_int(long num);
  INLINE void add_bool(bool flag);
  INLINE void add_fp(float num, float threshold);
  INLINE void add_fp(double num, double threshold);
  INLINE void add_pointer(void *ptr);
  void add_string(const std::string &str);
};

#include "checksumHashGenerator.I"

#endif
