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
#include "pStatClientImpl.h"

/**
 * This must be called at the start of every "frame", whatever a frame may be
 * deemed to be, to accumulate all the stats that have collected so far for
 * the thread and ship them off to the server.
 *
 * Calling PStatClient::thread_tick() will automatically call this for any
 * threads with the indicated sync name.
 */
void PStatThread::
new_frame(int frame_number) {
#ifdef DO_PSTATS
  _client->get_impl()->new_frame(_index, frame_number);
#endif
}

/**
 * This is a slightly lower-level version of new_frame that also specifies the
 * data to send for this frame.
 */
void PStatThread::
add_frame(int frame_number, PStatFrameData &&frame_data) {
#ifdef DO_PSTATS
  _client->get_impl()->add_frame(_index, frame_number, std::move(frame_data));
#endif
}

/**
 * Returns the Panda Thread object associated with this particular
 * PStatThread.
 */
Thread *PStatThread::
get_thread() const {
#ifdef DO_PSTATS
  return _client->get_thread_object(_index);
#else
  return Thread::get_current_thread();
#endif
}
