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
}

/**
 * This constructor is used to create the particular Thread object for each
 * external thread that is bound via Thread::bind_thread().
 */
ExternalThread::
ExternalThread(const std::string &name, const std::string &sync_name) :
  Thread(name, sync_name)
{
  _started = true;
}

/**
 *
 */
void ExternalThread::
thread_main() {
}
