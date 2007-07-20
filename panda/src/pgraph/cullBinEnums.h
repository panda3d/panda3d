// Filename: cullBinEnums.h
// Created by:  drose (03Apr06)
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

