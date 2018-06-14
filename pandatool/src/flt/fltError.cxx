/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file fltError.cxx
 * @author drose
 * @date 2000-08-24
 */

#include "fltError.h"

std::ostream &
operator << (std::ostream &out, FltError error) {
  switch (error) {
  case FE_ok:
    return out << "no error";

  case FE_could_not_open:
    return out << "could not open file";

  case FE_empty_file:
    return out << "empty file";

  case FE_end_of_file:
    return out << "unexpected end of file";

  case FE_read_error:
    return out << "read error on file";

  case FE_invalid_record:
    return out << "invalid record";

  case FE_extra_data:
    return out << "extra data at end of file";

  case FE_write_error:
    return out << "write error on file";

  case FE_bad_data:
    return out << "bad data";

  case FE_not_implemented:
    return out << "not implemented";

  case FE_internal:
    return out << "internal error";

  default:
    return out << "unknown error " << (int)error;
  }
}
