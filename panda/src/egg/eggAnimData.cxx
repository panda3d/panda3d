// Filename: eggAnimData.cxx
// Created by:  drose (19Feb99)
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
