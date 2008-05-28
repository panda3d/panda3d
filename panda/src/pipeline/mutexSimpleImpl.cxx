// Filename: mutexSimpleImpl.cxx
// Created by:  drose (19Jun07)
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

#ifdef THREAD_SIMPLE_IMPL

#include "mutexSimpleImpl.h"
#include "threadSimpleImpl.h"
#include "threadSimpleManager.h"

////////////////////////////////////////////////////////////////////
//     Function: MutexSimpleImpl::do_lock
//       Access: Private
//  Description: 
////////////////////////////////////////////////////////////////////
void MutexSimpleImpl::
do_lock() {
  ThreadSimpleManager *manager = ThreadSimpleManager::get_global_ptr();
  ThreadSimpleImpl *thread = manager->get_current_thread();

  while ((_flags & F_lock_count) != 0) {
    manager->enqueue_block(thread, this);
    manager->next_context();
  }
  
  _flags |= F_lock_count;
}

////////////////////////////////////////////////////////////////////
//     Function: MutexSimpleImpl::do_release
//       Access: Private
//  Description: 
////////////////////////////////////////////////////////////////////
void MutexSimpleImpl::
do_release() {
  ThreadSimpleManager *manager = ThreadSimpleManager::get_global_ptr();
  manager->unblock_one(this);
}

#endif  // THREAD_SIMPLE_IMPL
