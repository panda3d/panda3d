// Filename: timerQueryContext.cxx
// Created by:  rdb (22Aug14)
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

#include "timerQueryContext.h"

TypeHandle TimerQueryContext::_type_handle;

////////////////////////////////////////////////////////////////////
//     Function: TimerQueryContext::get_timestamp
//       Access: Public, Virtual
//  Description: Returns the timestamp that is the result of this
//               timer query.  There's no guarantee about which
//               clock this uses, the only guarantee is that
//               subtracting a start time from an end time should
//               yield a time in seconds.
//               If is_answer_ready() did not return true, this
//               function may block before it returns.
//
//               It is only valid to call this from the draw thread.
////////////////////////////////////////////////////////////////////
double TimerQueryContext::
get_timestamp() const {
  return 0.0;
}
