// Filename: hashGenerator.h
// Created by:  drose (22Mar01)
// 
////////////////////////////////////////////////////////////////////

#ifndef DCHASHGENERATOR_H
#define DCHASHGENERATOR_H

#include "dcbase.h"
#include "primeNumberGenerator.h"

////////////////////////////////////////////////////////////////////
// 	 Class : HashGenerator
// Description : This class generates an arbitrary hash number from a
//               sequence of ints.
////////////////////////////////////////////////////////////////////
class HashGenerator {
public:
  HashGenerator();

  void add_int(int num);
  void add_string(const string &str);

  unsigned long get_hash() const;

private:
  long _hash;
  int _index;
  PrimeNumberGenerator _primes;
};

#endif
