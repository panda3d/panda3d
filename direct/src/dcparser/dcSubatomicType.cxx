/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file dcSubatomicType.cxx
 * @author drose
 * @date 2000-10-05
 */

#include "dcSubatomicType.h"

std::ostream &
operator << (std::ostream &out, DCSubatomicType type) {
  switch (type) {
  case ST_int8:
    return out << "int8";

  case ST_int16:
    return out << "int16";

  case ST_int32:
    return out << "int32";

  case ST_int64:
    return out << "int64";

  case ST_uint8:
    return out << "uint8";

  case ST_uint16:
    return out << "uint16";

  case ST_uint32:
    return out << "uint32";

  case ST_uint64:
    return out << "uint64";

  case ST_float64:
    return out << "float64";

  case ST_string:
    return out << "string";

  case ST_blob:
    return out << "blob";

  case ST_blob32:
    return out << "blob32";

  case ST_int8array:
    return out << "int8array";

  case ST_int16array:
    return out << "int16array";

  case ST_int32array:
    return out << "int32array";

  case ST_uint8array:
    return out << "uint8array";

  case ST_uint16array:
    return out << "uint16array";

  case ST_uint32array:
    return out << "uint32array";

  case ST_uint32uint8array:
    return out << "uint32uint8array";

  case ST_char:
    return out << "char";

  case ST_invalid:
    return out << "invalid";
  }

  return out << "invalid type: " << (int)type;
}
