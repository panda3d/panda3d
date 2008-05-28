// Filename: primeNumberGenerator.h
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

#ifndef PRIMENUMBERGENERATOR_H
#define PRIMENUMBERGENERATOR_H

#include "dcbase.h"

#ifdef WITHIN_PANDA
// We only have the vector_int header file if we're compiling this
// package within the normal Panda environment.
#include "vector_int.h"

#else
typedef vector<int> vector_int;
#endif

////////////////////////////////////////////////////////////////////
//       Class : PrimeNumberGenerator
// Description : This class generates a table of prime numbers, up to
//               the limit of an int.  For a given integer n, it will
//               return the nth prime number.  This will involve a
//               recompute step only if n is greater than any previous
//               n.
////////////////////////////////////////////////////////////////////
class PrimeNumberGenerator {
public:
  PrimeNumberGenerator();

  int operator [] (int n);

private:
  typedef vector_int Primes;
  Primes _primes;
};

#endif
