// Filename: eggAnimData.cxx
// Created by:  drose (19Feb99)
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

#include "eggAnimData.h"

TypeHandle EggAnimData::_type_handle;

////////////////////////////////////////////////////////////////////
//     Function: EggAnimData::quantize
//       Access: Public
//  Description: Rounds each element of the table to the nearest
//               multiple of quantum.
////////////////////////////////////////////////////////////////////
void EggAnimData::
quantize(double quantum) {
  for (size_t i = 0; i < _data.size(); i++) {
    _data[i] = floor(_data[i] / quantum + 0.5) * quantum;
  }
}
