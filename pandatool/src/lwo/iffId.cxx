// Filename: iffId.cxx
// Created by:  drose (23Apr01)
// 
////////////////////////////////////////////////////////////////////

#include "iffId.h"

#include <ctype.h>

////////////////////////////////////////////////////////////////////
//     Function: IffId::output
//       Access: Public
//  Description: 
////////////////////////////////////////////////////////////////////
void IffId::
output(ostream &out) const {
  // If all of the characters are printable, just output them.  
  if (isprint(_id._c[0]) && isprint(_id._c[1]) &&
      isprint(_id._c[2]) && isprint(_id._c[3])) {
    out << _id._c[0] << _id._c[1] << _id._c[2] << _id._c[3];

  } else if (isprint(_id._c[0]) && isprint(_id._c[1]) &&
	     isprint(_id._c[2]) && _id._c[3] == '\0') {
    // If the last character is 0, output a 3-letter ID.
    out << _id._c[0] << _id._c[1] << _id._c[2];

  } else {
    // Otherwise, write out the hex.
    out << "0x" << hex << setfill('0');
    for (int i = 0; i < 4; i++) {
      out << setw(2) << (int)(unsigned char)_id._c[i];
    }
    out << dec << setfill(' ');
  }
}
