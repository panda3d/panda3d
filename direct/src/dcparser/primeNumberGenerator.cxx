// Filename: primeNumberGenerator.cxx
// Created by:  drose (22Mar01)
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

#include "primeNumberGenerator.h"


////////////////////////////////////////////////////////////////////
//     Function: PrimeNumberGenerator::Constructor
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
PrimeNumberGenerator::
PrimeNumberGenerator() {
  _primes.push_back(2);
}

////////////////////////////////////////////////////////////////////
//     Function: PrimeNumberGenerator::Indexing operator
//       Access: Public
//  Description: Returns the nth prime number.  this[0] returns 2,
//               this[1] returns 3; successively larger values of n
//               return larger prime numbers, up to the largest prime
//               number that can be represented in an int.
////////////////////////////////////////////////////////////////////
int PrimeNumberGenerator::
operator [] (int n) {
  nassertr(n >= 0, 0);

  // Compute the prime numbers between the last-computed prime number
  // and n.
  int candidate = _primes.back() + 1;
  while ((int)_primes.size() <= n) {
    // Is candidate prime?  It is not if any one of the already-found
    // prime numbers (up to its square root) divides it evenly.
    bool maybe_prime = true;
    int j = 0;
    while (maybe_prime && _primes[j] * _primes[j] <= candidate) {
      if ((_primes[j] * (candidate / _primes[j])) == candidate) {
        // This one is not prime.
        maybe_prime = false;
      }
      j++;
      nassertr(j < (int)_primes.size(), 0);
    }
    if (maybe_prime) {
      // Hey, we found a prime!
      _primes.push_back(candidate);
    }
    candidate++;
  }

  return _primes[n];
}
