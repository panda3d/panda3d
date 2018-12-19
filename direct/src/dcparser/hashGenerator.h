/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file hashGenerator.h
 * @author drose
 * @date 2001-03-22
 */

#ifndef DCHASHGENERATOR_H
#define DCHASHGENERATOR_H

#include "dcbase.h"
#include "primeNumberGenerator.h"
#include "vector_uchar.h"

/**
 * This class generates an arbitrary hash number from a sequence of ints.
 */
class EXPCL_DIRECT_DCPARSER HashGenerator {
public:
  HashGenerator();

  void add_int(int num);
  void add_string(const std::string &str);
  void add_blob(const vector_uchar &bytes);

  unsigned long get_hash() const;

private:
  long _hash;
  int _index;
  PrimeNumberGenerator _primes;
};

#endif
