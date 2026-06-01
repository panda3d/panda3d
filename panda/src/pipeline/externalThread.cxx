/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file externalThread.cxx
 * @author drose
 * @date 2006-01-30
 */

#include "externalThread.h"

TypeHandle ExternalThread::_type_handle;

/**
 * This constructor is used to create the one global ExternalThread object
 * that is shared by all externally-created threads that are not specifically
 * bound to a particular Thread object.
 */
ExternalThread::
ExternalThread() : Thread("External", "External") {
  init_type();  // in case static init comes in the wrong order
  _started = true;
#ifdef THREADED_PIPELINE
  // The shared singleton stands in for all unbound external threads; it takes
  // its single stage occupancy when created (already "running", unlike a
  // started thread that acquires in its root wrapper).
  acquire_stage_occupancy();
#endif
}

/**
 * This constructor is used to create the particular Thread object for each
 * external thread that is bound via Thread::bind_thread().
 */
ExternalThread::
ExternalThread(std::string name, std::string sync_name) :
  Thread(std::move(name), std::move(sync_name))
{
  _started = true;
#ifdef THREADED_PIPELINE
  // Bound external threads are already running; acquire occupancy in the ctor,
  // which runs on the external thread being bound.
  acquire_stage_occupancy();
#endif
}

/**
 *
 */
void ExternalThread::
thread_main() {
}
