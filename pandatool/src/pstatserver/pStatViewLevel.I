// Filename: pStatViewLevel.I
// Created by:  drose (19Jul00)
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
//     Function: PStatViewLevel::get_collector
//       Access: Public
//  Description: Returns the Collector index associated with this
//               level.
////////////////////////////////////////////////////////////////////
INLINE int PStatViewLevel::
get_collector() const {
  return _collector;
}

////////////////////////////////////////////////////////////////////
//     Function: PStatViewLevel::get_value_alone
//       Access: Public
//  Description: Returns the total level value (or elapsed time value)
//               for this Collector, not including any values
//               accounted for by its child Collectors.
////////////////////////////////////////////////////////////////////
INLINE double PStatViewLevel::
get_value_alone() const {
  return _value_alone;
}
