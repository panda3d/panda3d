/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file bamEnums.h
 * @author drose
 * @date 2009-02-26
 */

#ifndef BAMENUMS_H
#define BAMENUMS_H

#include "pandabase.h"

/**
 * This class exists just to provide scoping for the enums shared by BamReader
 * and BamWriter.
 */
class EXPCL_PANDA_PUTIL BamEnums {
PUBLISHED:

  // This defines an enumerated type used to represent the endianness of
  // certain numeric values stored in a Bam file.  It really has only two
  // possible values, either BE_bigendian or BE_littleendian; but through a
  // preprocessor trick we also add BE_native, which is the same numerically
  // as whichever value the hardware supports natively.
  enum BamEndian {
    BE_bigendian = 0,
    BE_littleendian = 1,
#ifdef WORDS_BIGENDIAN
    BE_native = 0,
#else
    BE_native = 1,
#endif
  };

/*
 * This is the code written along with each object.  It is used to control
 * object scoping.  A BOC_push includes an object definition, and will always
 * be eventually paired with a BOC_pop (which does not).  A BOC_adjunct
 * includes an object definition but does not push the level; it is associated
 * with the current level.  BOC_remove lists object ID's that have been
 * deallocated on the sender end.  BOC_file_data may appear at any level and
 * indicates the following datagram contains auxiliary file data that may be
 * referenced by a later object.
 */
  enum BamObjectCode {
    BOC_push,
    BOC_pop,
    BOC_adjunct,
    BOC_remove,
    BOC_file_data,
  };

  // This enum is used to control how textures are written to a bam stream.
  enum BamTextureMode {
    BTM_unchanged,
    BTM_fullpath,
    BTM_relative,
    BTM_basename,
    BTM_rawdata
  };
};

EXPCL_PANDA_PUTIL std::ostream &operator << (std::ostream &out, BamEnums::BamEndian be);
EXPCL_PANDA_PUTIL std::istream &operator >> (std::istream &in, BamEnums::BamEndian &be);

EXPCL_PANDA_PUTIL std::ostream &operator << (std::ostream &out, BamEnums::BamObjectCode boc);

EXPCL_PANDA_PUTIL std::ostream &operator << (std::ostream &out, BamEnums::BamTextureMode btm);
EXPCL_PANDA_PUTIL std::istream &operator >> (std::istream &in, BamEnums::BamTextureMode &btm);

#endif
