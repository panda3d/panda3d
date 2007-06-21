// Filename: conditionVarSimpleImpl.cxx
// Created by:  drose (19Jun07)
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

#include "conditionVarSimpleImpl.h"
#include "threadSimpleImpl.h"

////////////////////////////////////////////////////////////////////
//     Function: ConditionVarSimpleImpl::wait
//       Access: Public
//  Description: 
////////////////////////////////////////////////////////////////////
void ConditionVarSimpleImpl::
wait() {
  _mutex.release();

  ThreadSimpleManager *manager = ThreadSimpleManager::get_global_ptr();
  ThreadSimpleImpl *thread = manager->get_current_thread();
  manager->enqueue_block(thread, this);
  manager->next_context();

  _mutex.lock();
}

////////////////////////////////////////////////////////////////////
//     Function: ConditionVarSimpleImpl::do_signal
//       Access: Private
//  Description: 
////////////////////////////////////////////////////////////////////
void ConditionVarSimpleImpl::
do_signal() {
  ThreadSimpleManager *manager = ThreadSimpleManager::get_global_ptr();
  manager->unblock_one(this);
}

////////////////////////////////////////////////////////////////////
//     Function: ConditionVarSimpleImpl::do_signal_all
//       Access: Private
//  Description: 
////////////////////////////////////////////////////////////////////
void ConditionVarSimpleImpl::
do_signal_all() {
  ThreadSimpleManager *manager = ThreadSimpleManager::get_global_ptr();
  manager->unblock_all(this);
}

#endif  // THREAD_SIMPLE_IMPL
