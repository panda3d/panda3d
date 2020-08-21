/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file bamEnums.cxx
 * @author drose
 * @date 2009-02-26
 */

#include "bamEnums.h"
#include "string_utils.h"
#include "config_putil.h"

using std::istream;
using std::ostream;
using std::string;

ostream &
operator << (ostream &out, BamEnums::BamEndian be) {
  switch (be) {
  case BamEnums::BE_bigendian:
    return out << "bigendian";

  case BamEnums::BE_littleendian:
    return out << "littleendian";
  }

  return out << "**invalid BamEnums::BamEndian value: (" << (int)be << ")**";
}

istream &
operator >> (istream &in, BamEnums::BamEndian &be) {
  string word;
  in >> word;
  if (cmp_nocase(word, "native") == 0) {
    be = BamEnums::BE_native;

  } else if (cmp_nocase(word, "bigendian") == 0) {
    be = BamEnums::BE_bigendian;

  } else if (cmp_nocase(word, "littleendian") == 0) {
    be = BamEnums::BE_littleendian;

  } else {
    util_cat->error()
      << "Invalid bam_endian string: " << word << "\n";
    be = BamEnums::BE_native;
  }

  return in;
}


ostream &
operator << (ostream &out, BamEnums::BamObjectCode boc) {
  switch (boc) {
  case BamEnums::BOC_push:
    return out << "push";

  case BamEnums::BOC_pop:
    return out << "pop";

  case BamEnums::BOC_adjunct:
    return out << "adjunct";

  case BamEnums::BOC_remove:
    return out << "remove";

  case BamEnums::BOC_file_data:
    return out << "file_data";
  }

  return out << "**invalid BamEnums::BamObjectCode value: (" << (int)boc << ")**";
}

ostream &
operator << (ostream &out, BamEnums::BamTextureMode btm) {
  switch (btm) {
  case BamEnums::BTM_unchanged:
    return out << "unchanged";

  case BamEnums::BTM_fullpath:
    return out << "fullpath";

  case BamEnums::BTM_relative:
    return out << "relative";

  case BamEnums::BTM_basename:
    return out << "basename";

  case BamEnums::BTM_rawdata:
    return out << "rawdata";
  }

  return out << "**invalid BamEnums::BamTextureMode (" << (int)btm << ")**";
}

istream &
operator >> (istream &in, BamEnums::BamTextureMode &btm) {
  string word;
  in >> word;

  if (cmp_nocase(word, "unchanged") == 0) {
    btm = BamEnums::BTM_unchanged;
  } else if (cmp_nocase(word, "fullpath") == 0) {
    btm = BamEnums::BTM_fullpath;
  } else if (cmp_nocase(word, "relative") == 0) {
    btm = BamEnums::BTM_relative;
  } else if (cmp_nocase(word, "basename") == 0) {
    btm = BamEnums::BTM_basename;
  } else if (cmp_nocase(word, "rawdata") == 0) {
    btm = BamEnums::BTM_rawdata;

  } else {
    util_cat->error() << "Invalid BamEnums::BamTextureMode value: " << word << "\n";
    btm = BamEnums::BTM_relative;
  }

  return in;
}
