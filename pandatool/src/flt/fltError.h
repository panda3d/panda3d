// Filename: fltError.h
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

ostream &operator << (ostream &out, FltError error);

#endif



