// Filename: bamEndian.h
// Created by:  drose (07Jun05)
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

EXPCL_PANDA ostream &operator << (ostream &out, BamEndian be);
EXPCL_PANDA istream &operator >> (istream &in, BamEndian &be);

#endif
