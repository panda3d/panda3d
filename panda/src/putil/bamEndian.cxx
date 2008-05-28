// Filename: bamEndian.cxx
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

#include "bamEndian.h"
#include "string_utils.h"
#include "config_util.h"

ostream &
operator << (ostream &out, BamEndian be) {
  switch (be) {
  case BE_bigendian:
    return out << "bigendian";
   
  case BE_littleendian:
    return out << "littleendian";
  }

  return out << "**invalid BamEndian value: (" << (int)be << ")**";
}

istream &
operator >> (istream &in, BamEndian &be) {
  string word;
  in >> word;
  if (cmp_nocase(word, "native") == 0) {
    be = BE_native;

  } else if (cmp_nocase(word, "bigendian") == 0) {
    be = BE_bigendian;

  } else if (cmp_nocase(word, "littleendian") == 0) {
    be = BE_littleendian;

  } else {
    util_cat->error()
      << "Invalid bam_endian string: " << word << "\n";
    be = BE_native;
  }

  return in;
}
