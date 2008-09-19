// Filename: conditionVarSimpleImpl.cxx
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
//     Function: ConditionVarSimpleImpl::wait
//       Access: Public
//  Description: 
////////////////////////////////////////////////////////////////////
void ConditionVarSimpleImpl::
wait(double timeout) {
  _mutex.release();

  // TODO.  For now this will release every frame, since we don't have
  // an interface yet on ThreadSimpleManager to do a timed wait.
  // Maybe that's good enough forever (it does satisfy the condition
  // variable semantics, after all).
  ThreadSimpleManager *manager = ThreadSimpleManager::get_global_ptr();
  ThreadSimpleImpl *thread = manager->get_current_thread();
  manager->enqueue_ready(thread);
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
