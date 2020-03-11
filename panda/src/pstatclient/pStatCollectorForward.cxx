/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file pStatCollectorForward.cxx
 * @author drose
 * @date 2006-10-30
 */

#include "pStatCollectorForward.h"

/**
 *
 */
void PStatCollectorForward::
add_level(double increment) {
#ifdef DO_PSTATS
  _col.add_level_now(increment);
#endif  // DO_PSTATS
}
