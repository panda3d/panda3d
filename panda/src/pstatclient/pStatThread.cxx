/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file pStatThread.cxx
 * @author drose
 * @date 2006-01-30
 */

#include "pStatThread.h"
#include "pStatClient.h"

////////////////////////////////////////////////////////////////////
//     Function: PStatThread::get_thread
//       Access: Published
//  Description: Returns the Panda Thread object associated with this
//               particular PStatThread.
////////////////////////////////////////////////////////////////////
Thread *PStatThread::
get_thread() const {
#ifdef DO_PSTATS
  return _client->get_thread_object(_index);
#else
  return Thread::get_current_thread();
#endif
}
