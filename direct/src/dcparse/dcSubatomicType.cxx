// Filename: dcSubatomicType.cxx
// Created by:  drose (05Oct00)
// 
////////////////////////////////////////////////////////////////////

#include "dcSubatomicType.h"

ostream &
operator << (ostream &out, DCSubatomicType type) {
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

  case ST_int16array:
    return out << "int16array";

  case ST_int32array:
    return out << "int32array";

  case ST_uint16array:
    return out << "uint16array";

  case ST_uint32array:
    return out << "uint32array";
  }

  return out << "invalid type: " << (int)type;
}
