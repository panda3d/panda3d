/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file cullBinEnums.h
 * @author drose
 * @date 2006-04-03
 */

#ifndef CULLBINENUMS_H
#define CULLBINENUMS_H

#include "pandabase.h"

/**
 * Provides scoping for the enumerated type shared by CullBin and
 * CullBinManager.
 */
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
