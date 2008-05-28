// Filename: pStatCollectorForward.cxx
// Created by:  drose (30Oct06)
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

#include "pStatCollectorForward.h"

#ifdef DO_PSTATS
////////////////////////////////////////////////////////////////////
//     Function: PStatCollectorForward::add_level
//       Access: Public, Virtual
//  Description:
////////////////////////////////////////////////////////////////////
void PStatCollectorForward::
add_level(double increment) {
  _col.add_level_now(increment);
}
#endif  // DO_PSTATS

