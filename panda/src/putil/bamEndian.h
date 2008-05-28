// Filename: bamEndian.h
// Created by:  drose (07Jun05)
//
////////////////////////////////////////////////////////////////////
//
// PANDA 3D SOFTWARE
// Copyright (c) Carnegie Mellon University.  All rights reserved.
//
// All use of this software is subject to the terms of the revised BSD
// license.  You should have received a copy of this license along
// with this source code in a file named "LICENSE."
//
////////////////////////////////////////////////////////////////////

#ifndef BAMENDIAN_H
#define BAMENDIAN_H

#include "pandabase.h"

// This defines an enumerated type used to represent the endianness of
// certain numeric values stored in a Bam file.  It really has only
// two possible values, either BE_bigendian or BE_littleendian; but
// through a preprocessor trick we also add BE_native, which is the
// same numerically as whichever value the hardware supports natively.
enum BamEndian {
  BE_bigendian = 0,
  BE_littleendian = 1,
#ifdef WORDS_BIGENDIAN
  BE_native = 0,
#else
  BE_native = 1,
#endif
};

EXPCL_PANDA_PUTIL ostream &operator << (ostream &out, BamEndian be);
EXPCL_PANDA_PUTIL istream &operator >> (istream &in, BamEndian &be);

#endif
