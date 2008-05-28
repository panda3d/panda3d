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
