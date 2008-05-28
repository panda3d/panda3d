// Filename: occlusionQueryContext.cxx
// Created by:  drose (27Mar06)
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

#include "occlusionQueryContext.h"

TypeHandle OcclusionQueryContext::_type_handle;

////////////////////////////////////////////////////////////////////
//     Function: OcclusionQueryContext::get_num_fragments
//       Access: Public, Virtual
//  Description: Returns the number of fragments (pixels) of the
//               specified geometry that passed the depth test.
//               If is_answer_ready() did not return true, this
//               function may block before it returns.
//
//               It is only valid to call this from the draw thread.
////////////////////////////////////////////////////////////////////
int OcclusionQueryContext::
get_num_fragments() const {
  return 0;
}
