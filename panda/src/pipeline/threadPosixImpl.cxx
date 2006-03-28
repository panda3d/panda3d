// Filename: threadPosixImpl.cxx
// Created by:  drose (09Feb06)
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

#include "threadPosixImpl.h"
#include "selectThreadImpl.h"

#ifdef THREAD_POSIX_IMPL

#include "pointerTo.h"
#include "config_pipeline.h"

#include <sched.h>

pthread_key_t ThreadPosixImpl::_pt_ptr_index = 0;
bool ThreadPosixImpl::_got_pt_ptr_index = false;

////////////////////////////////////////////////////////////////////
//     Function: ThreadPosixImpl::Destructor
//       Access: Public
//  Description: 
////////////////////////////////////////////////////////////////////
ThreadPosixImpl::
~ThreadPosixImpl() {
  if (thread_cat.is_debug()) {
    thread_cat.debug() 
      << "Deleting thread " << _parent_obj->get_name() << "\n";
  }

  _mutex.lock();

  if (!_detached) {
    pthread_detach(_thread);
    _detached = true;
  }

  _mutex.release();
}

////////////////////////////////////////////////////////////////////
//     Function: ThreadPosixImpl::start
//       Access: Public
//  Description: 
////////////////////////////////////////////////////////////////////
bool ThreadPosixImpl::
start(ThreadPriority priority, bool global, bool joinable) {
  _mutex.lock();
  if (thread_cat.is_debug()) {
    thread_cat.debug() << "Starting thread " << _parent_obj->get_name() << "\n";
  }

  nassertd(_status == S_new) {
    _mutex.release();
    return false;
  }

  _joinable = joinable;
  _status = S_start_called;
  _detached = false;

  if (!_got_pt_ptr_index) {
    init_pt_ptr_index();
  }

  pthread_attr_t attr;
  pthread_attr_init(&attr);

  if (!_joinable) {
    pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_DETACHED);
    _detached = true;
  }

  // Ensure the thread has "system" scope, which should ensure it can
  // run in parallel with other threads.
  int result = pthread_attr_setscope(&attr, PTHREAD_SCOPE_SYSTEM);
  if (result != 0) {
    thread_cat.warning()
      << "Unable to set system scope.\n";
  }

  struct sched_param param;
  int current_policy = SCHED_OTHER;
  result = pthread_attr_setschedpolicy(&attr, SCHED_OTHER);
  if (result != 0) {
    thread_cat.warning()
      << "Unable to set scheduling policy.\n";

  }

  result = 0;
  switch (priority) {
  case TP_low:
    param.sched_priority = sched_get_priority_min(current_policy);
    result = pthread_attr_setschedparam(&attr, &param);
    break;
    
  case TP_high:
  case TP_urgent:
    param.sched_priority = sched_get_priority_max(current_policy);
    result = pthread_attr_setschedparam(&attr, &param);
    break;
    
  case TP_normal:
  default:
    break;
  }
  
  if (result != 0) {
    thread_cat.warning()
      << "Unable to specify thread priority.\n";
  }

  // Increment the parent object's reference count first.  The thread
  // will eventually decrement it when it terminates.
  _parent_obj->ref();
  result = pthread_create(&_thread, &attr, &root_func, (void *)this);

  pthread_attr_destroy(&attr);

  if (result != 0) {
    // Oops, we couldn't start the thread.  Be sure to decrement the
    // reference count we incremented above, and return false to
    // indicate failure.
    unref_delete(_parent_obj);
    _mutex.release();
    return false;
  }

  // Thread was successfully started.
  _mutex.release();
  return true;
}

////////////////////////////////////////////////////////////////////
//     Function: ThreadPosixImpl::interrupt
//       Access: Public
//  Description: Sends an interrupt message to the thread.  This will
//               interrupt any blocking-type system calls the thread
//               may be waiting on, such as I/O, so that the thread
//               may continue some other processing.  The specific
//               behavior is implementation dependent.
////////////////////////////////////////////////////////////////////
void ThreadPosixImpl::
interrupt() {
}

////////////////////////////////////////////////////////////////////
//     Function: ThreadPosixImpl::join
//       Access: Public
//  Description: Blocks the calling process until the thread
//               terminates.  If the thread has already terminated,
//               this returns immediately.
////////////////////////////////////////////////////////////////////
void ThreadPosixImpl::
join() {
  _mutex.lock();
  if (!_detached) {
    _mutex.release();
    void *return_val;
    pthread_join(_thread, &return_val);
    _detached = true;
    return;
  }
  _mutex.release();
}

////////////////////////////////////////////////////////////////////
//     Function: ThreadPosixImpl::root_func
//       Access: Private, Static
//  Description: The entry point of each thread.
////////////////////////////////////////////////////////////////////
void *ThreadPosixImpl::
root_func(void *data) {
  ThreadPosixImpl *self = (ThreadPosixImpl *)data;
  int result = pthread_setspecific(_pt_ptr_index, self->_parent_obj);
  nassertr(result == 0, NULL);

  {
    self->_mutex.lock();
    nassertd(self->_status == S_start_called) {
      self->_mutex.release();
      return NULL;
    }
      
    self->_status = S_running;
    self->_mutex.release();
  }

  self->_parent_obj->thread_main();

  if (thread_cat.is_debug()) {
    thread_cat.debug()
      << "Terminating thread " << self->_parent_obj->get_name() 
      << ", count = " << self->_parent_obj->get_ref_count() << "\n";
  }

  {
    self->_mutex.lock();
    nassertd(self->_status == S_running) {
      self->_mutex.release();
      return NULL;
    }
    self->_status = S_finished;
    self->_mutex.release();
  }

  // Now drop the parent object reference that we grabbed in start().
  // This might delete the parent object, and in turn, delete the
  // ThreadPosixImpl object.
  unref_delete(self->_parent_obj);

  return NULL;
}

////////////////////////////////////////////////////////////////////
//     Function: ThreadPosixImpl::init_pt_ptr_index
//       Access: Private, Static
//  Description: Allocate a new index to store the Thread parent
//               pointer as a piece of per-thread private data.
////////////////////////////////////////////////////////////////////
void ThreadPosixImpl::
init_pt_ptr_index() {
  nassertv(!_got_pt_ptr_index);

  int result = pthread_key_create(&_pt_ptr_index, NULL);
  if (result != 0) {
    thread_cat.error()
      << "Unable to associate Thread pointers with threads.\n";
    return;
  }

  _got_pt_ptr_index = true;

  // Assume that we must be in the main thread, since this method must
  // be called before the first thread is spawned.
  Thread *main_thread_obj = Thread::get_main_thread();
  result = pthread_setspecific(_pt_ptr_index, main_thread_obj);
  nassertv(result == 0);
}

#endif  // THREAD_POSIX_IMPL
