// Filename: pStatViewLevel.I
// Created by:  drose (19Jul00)
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
INLINE float PStatViewLevel::
get_value_alone() const {
  return _value_alone;
}
