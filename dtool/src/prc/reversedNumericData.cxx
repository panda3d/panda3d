// Filename: reversedNumericData.cxx
// Created by:  drose (09May01)
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


#include "pnotify.h"

#include "reversedNumericData.h"

////////////////////////////////////////////////////////////////////
//     Function: ReversedNumericData::reverse_assign
//       Access: Private
//  Description: Actually does the data reversal.
////////////////////////////////////////////////////////////////////
void ReversedNumericData::
reverse_assign(const char *source, size_t length) {
  nassertv((int)length <= max_numeric_size);
  for (size_t i = 0; i < length; i++) {
    _data[i] = source[length - 1 - i];
  }
}
