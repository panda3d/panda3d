// Filename: littleEndian.cxx
// Created by:  drose (09Feb00)
// 
////////////////////////////////////////////////////////////////////

#include "littleEndian.h"

#ifdef IS_BIG_ENDIAN

////////////////////////////////////////////////////////////////////
//     Function: LittleEndian::reverse_assign
//       Access: Private
//  Description: Actually does the data reversal.
////////////////////////////////////////////////////////////////////
void LittleEndian::
reverse_assign(const char *data, size_t length) {
  _str = "";
  _str.reserve(length);

  for (size_t i = 0; i < length; i++) {
    _str += data[length - 1 - i];
  }
}

#endif // IS_BIG_ENDIAN
