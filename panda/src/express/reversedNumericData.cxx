// Filename: reversedNumericData.cxx
// Created by:  drose (09May01)
// 
////////////////////////////////////////////////////////////////////

#include "reversedNumericData.h"

#include <notify.h>

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
