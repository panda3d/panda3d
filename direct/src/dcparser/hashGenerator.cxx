// Filename: hashGenerator.cxx
// Created by:  drose (22Mar01)
// 
////////////////////////////////////////////////////////////////////

#include "hashGenerator.h"
#include "primeNumberGenerator.h"

// We multiply each consecutive integer by the next prime number and
// add it to the total, so in theory we will truly generate a unique
// hash number for each unique sequence of ints, as long as the number
// of ints does not exceed the number of prime numbers we have, and we
// do not overflow the limits of a 32-bit integer.

// We do recycle the prime number table at some point, just to keep it
// from growing insanely large, however, and we also truncate
// everything to the low-order 32 bits, so we introduce ambiguity in
// this way.

static const int max_prime_numbers = 10000;
static PrimeNumberGenerator primes;

////////////////////////////////////////////////////////////////////
//     Function: HashGenerator::Constructor
//       Access: Public
//  Description: 
////////////////////////////////////////////////////////////////////
HashGenerator::
HashGenerator() {
  _hash = 0;
  _index = 0;
}

////////////////////////////////////////////////////////////////////
//     Function: HashGenerator::add_int
//       Access: Public
//  Description: Adds another integer to the hash so far.
////////////////////////////////////////////////////////////////////
void HashGenerator::
add_int(int num) {
  nassertv(_index >= 0 && _index < max_prime_numbers);
  _hash += (int)primes[_index] * num;
  _index = (_index + 1) % max_prime_numbers;
}

////////////////////////////////////////////////////////////////////
//     Function: HashGenerator::add_string
//       Access: Public
//  Description: Adds a string to the hash, by breaking it down into a
//               sequence of integers.
////////////////////////////////////////////////////////////////////
void HashGenerator::
add_string(const string &str) {
  add_int(str.length());
  string::const_iterator si;
  for (si = str.begin(); si != str.end(); ++si) {
    add_int(*si);
  }
}

////////////////////////////////////////////////////////////////////
//     Function: HashGenerator::get_hash
//       Access: Public
//  Description: Returns the hash number generated.
////////////////////////////////////////////////////////////////////
long HashGenerator::
get_hash() const {
  return _hash & 0xffffffff;
}
