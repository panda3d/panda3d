// Filename: asyncTaskBase.cxx
// Created by:  drose (09Feb10)
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

#include "asyncTaskBase.h"
#include "thread.h"
#include "atomicAdjust.h"

TypeHandle AsyncTaskBase::_type_handle;

////////////////////////////////////////////////////////////////////
//     Function: AsyncTaskBase::Constructor
//       Access: Protected
//  Description:
////////////////////////////////////////////////////////////////////
AsyncTaskBase::
AsyncTaskBase() {
}

////////////////////////////////////////////////////////////////////
//     Function: AsyncTaskBase::Destructor
//       Access: Published, Virtual
//  Description:
////////////////////////////////////////////////////////////////////
AsyncTaskBase::
~AsyncTaskBase() {
}

////////////////////////////////////////////////////////////////////
//     Function: AsyncTaskBase::record_task
//       Access: Protected
//  Description: Indicates that this task is now the current task
//               running on the indicated thread, presumably the
//               current thread.
////////////////////////////////////////////////////////////////////
void AsyncTaskBase::
record_task(Thread *current_thread) {
  nassertv(current_thread->_current_task == NULL);

  void *result = AtomicAdjust::compare_and_exchange_ptr
    ((void * TVOLATILE &)current_thread->_current_task,
     (void *)NULL, (void *)this);

  // If the return value is other than NULL, someone else must have
  // assigned the task first, in another thread.  That shouldn't be
  // possible.

  // But different versions of gcc appear to have problems compiling these
  // assertions correctly.
#ifndef __GNUC__
  nassertv(result == NULL);
  nassertv(current_thread->_current_task == this);
#endif  // __GNUC__
}

////////////////////////////////////////////////////////////////////
//     Function: AsyncTaskBase::clear_task
//       Access: Protected
//  Description: Indicates that this task is no longer running on the
//               indicated thread.
////////////////////////////////////////////////////////////////////
void AsyncTaskBase::
clear_task(Thread *current_thread) {
  nassertv(current_thread->_current_task == this);

  void *result = AtomicAdjust::compare_and_exchange_ptr
    ((void * TVOLATILE &)current_thread->_current_task,
     (void *)this, (void *)NULL);

  // If the return value is other than this, someone else must have
  // assigned the task first, in another thread.  That shouldn't be
  // possible.

  // But different versions of gcc appear to have problems compiling these
  // assertions correctly.
#ifndef __GNUC__
  nassertv(result == this);
  nassertv(current_thread->_current_task == NULL);
#endif  // __GNUC__
}
