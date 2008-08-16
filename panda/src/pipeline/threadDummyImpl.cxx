// Filename: threadDummyImpl.cxx
// Created by:  drose (09Aug02)
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

#include "selectThreadImpl.h"

#ifdef THREAD_DUMMY_IMPL

#include "threadDummyImpl.h"
#include "thread.h"

#ifdef WIN32
#define WIN32_LEAN_AND_MEAN 1
#include <windows.h>
#endif

////////////////////////////////////////////////////////////////////
//     Function: ThreadDummyImpl::get_unique_id
//       Access: Public
//  Description: 
////////////////////////////////////////////////////////////////////
string ThreadDummyImpl::
get_unique_id() const {
  // In a single-threaded application, this is just the unique process
  // ID.
  ostringstream strm;
#ifdef WIN32
  strm << GetCurrentProcessId();
#else
  strm << getpid();
#endif
  return strm.str();
}

////////////////////////////////////////////////////////////////////
//     Function: ThreadDummyImpl::get_current_thread
//       Access: Public
//  Description: 
////////////////////////////////////////////////////////////////////
Thread *ThreadDummyImpl::
get_current_thread() {
  return Thread::get_main_thread();
}

#endif  // THREAD_DUMMY_IMPL
