// Filename: fltError.cxx
// Created by:  drose (24Aug00)
//
////////////////////////////////////////////////////////////////////
//
// PANDA 3D SOFTWARE
// Copyright (c) 2001 - 2004, Disney Enterprises, Inc.  All rights reserved
//
// All use of this software is subject to the terms of the Panda 3d
// Software license.  You should have received a copy of this license
// along with this source code; you will also find a current copy of
// the license at http://etc.cmu.edu/panda3d/docs/license/ .
//
// To contact the maintainers of this program write to
// panda3d-general@lists.sourceforge.net .
//
////////////////////////////////////////////////////////////////////

#include "fltError.h"

ostream &
operator << (ostream &out, FltError error) {
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
