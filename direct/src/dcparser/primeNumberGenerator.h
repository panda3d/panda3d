// Filename: primeNumberGenerator.h
// Created by:  drose (22Mar01)
//
////////////////////////////////////////////////////////////////////
//
// PANDA 3D SOFTWARE
// Copyright (c) 2001, Disney Enterprises, Inc.  All rights reserved
//
// All use of this software is subject to the terms of the Panda 3d
// Software license.  You should have received a copy of this license
// along with this source code; you will also find a current copy of
// the license at http://www.panda3d.org/license.txt .
//
// To contact the maintainers of this program write to
// panda3d@yahoogroups.com .
//
////////////////////////////////////////////////////////////////////

#ifndef PRIMENUMBERGENERATOR_H
#define PRIMENUMBERGENERATOR_H

#include "dcbase.h"

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
  typedef pvector<int> Primes;
  Primes _primes;
};

#endif
