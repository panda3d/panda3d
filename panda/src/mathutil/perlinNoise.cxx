// Filename: perlinNoise.cxx
// Created by:  drose (05Oct05)
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

#include "perlinNoise.h"

Mersenne PerlinNoise::_next_seed(0);
bool PerlinNoise::_got_first_seed = false;

////////////////////////////////////////////////////////////////////
//     Function: PerlinNoise::Constructor
//       Access: Protected
//  Description: Randomizes the tables to make a unique noise
//               function.
//
//               If seed is nonzero, it is used to define the tables;
//               if it is zero a random seed is generated.
////////////////////////////////////////////////////////////////////
PerlinNoise::
PerlinNoise(int table_size, unsigned long seed) :
  _table_size(table_size),
  _mersenne(seed != 0 ? seed : get_next_seed())
{
  // The _index table is just a randomly shuffled index.
  // table.
  _index.reserve(_table_size * 2);
  int i;
  for (i = 0; i < _table_size; ++i) {
    _index.push_back(i);
  }
  for (i = 0; i < _table_size; ++i) {
    int j = random_int(_table_size);
    nassertv(j >= 0 && j < _table_size);
    int t = _index[i];
    _index[i] = _index[j];
    _index[j] = t;
  }

  // We double up _index so we don't need to perform modulo
  // arithmetic.
  for (i = 0; i < _table_size; ++i) {
    _index.push_back(_index[i]);
  }
}
