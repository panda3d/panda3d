// Filename: primeNumberGenerator.h
// Created by:  drose (22Mar01)
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
  typedef vector<int> Primes;
  Primes _primes;
};

#endif
