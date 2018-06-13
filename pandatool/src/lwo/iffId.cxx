/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file iffId.cxx
 * @author drose
 * @date 2001-04-23
 */

#include "iffId.h"

#include <ctype.h>

/**
 *
 */
void IffId::
output(std::ostream &out) const {
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
    out << "0x" << std::hex << std::setfill('0');
    for (int i = 0; i < 4; i++) {
      out << std::setw(2) << (int)(unsigned char)_id._c[i];
    }
    out << std::dec << std::setfill(' ');
  }
}
