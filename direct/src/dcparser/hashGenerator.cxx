/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file hashGenerator.cxx
 * @author drose
 * @date 2001-03-22
 */

#include "hashGenerator.h"
#include "primeNumberGenerator.h"

// We multiply each consecutive integer by the next prime number and add it to
// the total.  This will generate pretty evenly-distributed hash numbers for
// an arbitrary sequence of ints.

// We do recycle the prime number table at some point, just to keep it from
// growing insanely large, however (and to avoid wasting time computing large
// prime numbers unnecessarily), and we also truncate the result to the low-
// order 32 bits.

static const int max_prime_numbers = 10000;

/**
 *
 */
HashGenerator::
HashGenerator() {
  _hash = 0;
  _index = 0;
}

/**
 * Adds another integer to the hash so far.
 */
void HashGenerator::
add_int(int num) {
  nassertv(_index >= 0 && _index < max_prime_numbers);
  _hash += _primes[_index] * num;
  _index = (_index + 1) % max_prime_numbers;
}

/**
 * Adds a string to the hash, by breaking it down into a sequence of integers.
 */
void HashGenerator::
add_string(const std::string &str) {
  add_int(str.length());
  std::string::const_iterator si;
  for (si = str.begin(); si != str.end(); ++si) {
    add_int(*si);
  }
}

/**
 * Adds a blob to the hash, by breaking it down into a sequence of integers.
 */
void HashGenerator::
add_blob(const vector_uchar &bytes) {
  add_int(bytes.size());
  vector_uchar::const_iterator bi;
  for (bi = bytes.begin(); bi != bytes.end(); ++bi) {
    add_int(*bi);
  }
}

/**
 * Returns the hash number generated.
 */
unsigned long HashGenerator::
get_hash() const {
  return (unsigned long)(_hash & 0xffffffff);
}
