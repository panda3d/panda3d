// Filename: dataNodeTransmit.cxx
// Created by:  drose (11Mar02)
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

#include "dataNodeTransmit.h"

////////////////////////////////////////////////////////////////////
//     Function: DataNodeTransmit::slot_data
//       Access: Private
//  Description: Ensures that the given index number exists in the
//               data array.
////////////////////////////////////////////////////////////////////
void DataNodeTransmit::
slot_data(int index) {
  nassertv(index < 1000);
  while (index >= (int)_data.size()) {
    _data.push_back(EventParameter());
  }
}
