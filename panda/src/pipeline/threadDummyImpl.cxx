/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file threadDummyImpl.cxx
 * @author drose
 * @date 2002-08-09
 */

#include "selectThreadImpl.h"

#ifdef THREAD_DUMMY_IMPL

#include "threadDummyImpl.h"
#include "thread.h"

#ifdef _WIN32
#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN 1
#endif
#include <windows.h>
#endif

/**
 *
 */
std::string ThreadDummyImpl::
get_unique_id() const {
  // In a single-threaded application, this is just the unique process ID.
  std::ostringstream strm;
#ifdef WIN32
  strm << GetCurrentProcessId();
#else
  strm << getpid();
#endif
  return strm.str();
}

/**
 *
 */
Thread *ThreadDummyImpl::
get_current_thread() {
  return Thread::get_main_thread();
}

#endif  // THREAD_DUMMY_IMPL
