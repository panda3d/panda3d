// Filename: occlusionQueryContext.I
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


////////////////////////////////////////////////////////////////////
//     Function: TimerQueryContext::Constructor
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
INLINE TimerQueryContext::
TimerQueryContext(int pstats_index) :
  _pstats_index(pstats_index),
  _frame_index(ClockObject::get_global_clock()->get_frame_count())
{
}
