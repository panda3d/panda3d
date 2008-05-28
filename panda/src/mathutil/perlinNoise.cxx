// Filename: perlinNoise.cxx
// Created by:  drose (05Oct05)
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

#include "perlinNoise.h"

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
  _table_size_mask(table_size - 1),
  _randomizer(seed)
{
  // It is necessary for _table_size to be a power of 2.
#ifndef NDEBUG
  if (_table_size != 0) {
    bool table_size_power_2 = ((_table_size ^ _table_size_mask) == (_table_size + _table_size_mask));
    nassertd(table_size_power_2) {
      _table_size = 0;
      _table_size_mask = 0;
    }
  }
#endif  // NDEBUG

  // The _index table is just a randomly shuffled index
  // table.
  _index.reserve(_table_size * 2);
  int i;
  for (i = 0; i < _table_size; ++i) {
    _index.push_back(i);
  }
  for (i = 0; i < _table_size; ++i) {
    int j = _randomizer.random_int(_table_size);
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

////////////////////////////////////////////////////////////////////
//     Function: PerlinNoise::Copy Constructor
//       Access: Protected
//  Description: Makes an exact copy of the existing PerlinNoise
//               object, including its random seed.
////////////////////////////////////////////////////////////////////
PerlinNoise::
PerlinNoise(const PerlinNoise &copy) :
  _table_size(copy._table_size),
  _table_size_mask(copy._table_size_mask),
  _randomizer(copy._randomizer),
  _index(copy._index)
{
}

////////////////////////////////////////////////////////////////////
//     Function: PerlinNoise::Copy Assignment Operator
//       Access: Protected
//  Description: Makes an exact copy of the existing PerlinNoise
//               object, including its random seed.
////////////////////////////////////////////////////////////////////
void PerlinNoise::
operator = (const PerlinNoise &copy) {
  _table_size = copy._table_size;
  _table_size_mask = copy._table_size_mask;
  _randomizer = copy._randomizer;
  _index = copy._index;
}
