// Filename: hashGenerator.h
// Created by:  drose (22Mar01)
//
////////////////////////////////////////////////////////////////////
//
// PANDA 3D SOFTWARE
// Copyright (c) 2001 - 2004, Disney Enterprises, Inc.  All rights reserved
//
// All use of this software is subject to the terms of the Panda 3d
// Software license.  You should have received a copy of this license
// along with this source code; you will also find a current copy of
// the license at http://etc.cmu.edu/panda3d/docs/license/ .
//
// To contact the maintainers of this program write to
// panda3d-general@lists.sourceforge.net .
//
////////////////////////////////////////////////////////////////////

#ifndef DCHASHGENERATOR_H
#define DCHASHGENERATOR_H

#include "dcbase.h"
#include "primeNumberGenerator.h"

////////////////////////////////////////////////////////////////////
//       Class : HashGenerator
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
