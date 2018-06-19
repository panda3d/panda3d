/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file fltError.h
 * @author drose
 * @date 2000-08-24
 */

#ifndef FLTERROR_H
#define FLTERROR_H

#include "pandatoolbase.h"

// Return values for various functions in the flt library.
enum FltError {
  FE_ok = 0,
  FE_could_not_open,
  FE_empty_file,
  FE_end_of_file,
  FE_read_error,
  FE_invalid_record,
  FE_extra_data,
  FE_write_error,
  FE_bad_data,
  FE_not_implemented,
  FE_undefined_instance,
  FE_internal
};

std::ostream &operator << (std::ostream &out, FltError error);

#endif
