// Filename: reMutexSimpleImpl.cxx
// Created by:  drose (20Jun07)
//
////////////////////////////////////////////////////////////////////
//
// PANDA 3D SOFTWARE
// Copyright (c) 2001 - 2004, Disney Enterprises, Inc.  All rights reserved
//
// All use of this software is subject to the terms of the Panda 3d
// Software license.  You should have received a copy of this license
// along with this source code; you will also find a current copy of
// the license at http://etc.cmu.edu/panda3d/docs/license/ .
//
// To contact the maintainers of this program write to
// panda3d-general@lists.sourceforge.net .
//
////////////////////////////////////////////////////////////////////

#include "selectThreadImpl.h"

#ifdef THREAD_SIMPLE_IMPL

#include "reMutexSimpleImpl.h"
#include "threadSimpleImpl.h"
#include "threadSimpleManager.h"

////////////////////////////////////////////////////////////////////
//     Function: ReMutexSimpleImpl::do_lock
//       Access: Private
//  Description: 
////////////////////////////////////////////////////////////////////
void ReMutexSimpleImpl::
do_lock() {
  ThreadSimpleManager *manager = ThreadSimpleManager::get_global_ptr();
  ThreadSimpleImpl *thread = manager->get_current_thread();

  while ((_flags & F_lock_count) != 0) {
    manager->enqueue_block(thread, this);
    manager->next_context();
  }
  
  ++_flags;
  _locking_thread = thread;
}

////////////////////////////////////////////////////////////////////
//     Function: ReMutexSimpleImpl::do_release
//       Access: Private
//  Description: 
////////////////////////////////////////////////////////////////////
void ReMutexSimpleImpl::
do_release() {
  ThreadSimpleManager *manager = ThreadSimpleManager::get_global_ptr();
  manager->unblock_one(this);
}

#endif  // THREAD_SIMPLE_IMPL
