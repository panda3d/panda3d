// Filename: cullBinEnums.h
// Created by:  drose (03Apr06)
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

#ifndef CULLBINENUMS_H
#define CULLBINENUMS_H

#include "pandabase.h"

////////////////////////////////////////////////////////////////////
//       Class : CullBinEnums
// Description : Provides scoping for the enumerated type shared by
//               CullBin and CullBinManager.
////////////////////////////////////////////////////////////////////
class EXPCL_PANDA_PGRAPH CullBinEnums {
PUBLISHED:
  enum BinType {
    BT_invalid,
    BT_unsorted,
    BT_state_sorted,
    BT_back_to_front,
    BT_front_to_back,
    BT_fixed,
  };
};

#endif

