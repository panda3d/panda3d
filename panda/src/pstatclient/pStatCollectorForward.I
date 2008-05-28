// Filename: pStatCollectorForward.I
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


////////////////////////////////////////////////////////////////////
//     Function: PStatCollectorForward::Constructor
//       Access: Published
//  Description:
////////////////////////////////////////////////////////////////////
INLINE PStatCollectorForward::
PStatCollectorForward(const PStatCollector &col)
#ifdef DO_PSTATS
  : _col(col)
#endif  // DO_PSTATS
{
}

