// Filename: bigEndian.cxx
// Created by:  drose (23Aug00)
// 
////////////////////////////////////////////////////////////////////

#include "bigEndian.h"

#ifdef IS_LITTLE_ENDIAN

////////////////////////////////////////////////////////////////////
//     Function: BigEndian::reverse_assign
//       Access: Private
//  Description: Actually does the data reversal.
////////////////////////////////////////////////////////////////////
void BigEndian::
reverse_assign(const char *data, size_t length) {
  _str = "";
  _str.reserve(length);

  for (size_t i = 0; i < length; i++) {
    _str += data[length - 1 - i];
  }
}

#endif // IS_LITTLE_ENDIAN
