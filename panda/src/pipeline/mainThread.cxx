/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file mainThread.cxx
 * @author drose
 * @date 2006-01-15
 */

#include "mainThread.h"

TypeHandle MainThread::_type_handle;

/**
 *
 */
MainThread::
MainThread() : Thread("Main", "Main") {
  init_type();  // in case static init comes in the wrong order
  _impl.setup_main_thread();
  _started = true;
#ifdef THREADED_PIPELINE
  // The main thread is already running on this stack, so it occupies its stage
  // immediately (unlike a started thread, which acquires in its root wrapper).
  acquire_stage_occupancy();
#endif
}

/**
 *
 */
void MainThread::
thread_main() {
}
