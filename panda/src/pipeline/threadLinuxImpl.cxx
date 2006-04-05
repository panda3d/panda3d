// Filename: threadLinuxImpl.cxx
// Created by:  drose (28Mar06)
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

#include "threadLinuxImpl.h"
#include "selectThreadImpl.h"

#ifdef THREAD_LINUX_IMPL

#include "pointerTo.h"
#include "config_pipeline.h"

#include <sched.h>
#include <sys/types.h>
#include <linux/unistd.h>
#include <signal.h>
#include <stdio.h>   // for perror

MutexLinuxImpl ThreadLinuxImpl::_thread_pointers_lock;
ThreadLinuxImpl::ThreadPointers ThreadLinuxImpl::_thread_pointers;
bool ThreadLinuxImpl::_got_main_thread_pointer;

inline static pid_t gettid() {
#ifdef __i386__
  pid_t ret;
  __asm__("int $0x80" : "=a" (ret) : "0" (224) /* SYS_gettid */);
  return ret;
#else
#error only i386 supported right now.
#endif
}

////////////////////////////////////////////////////////////////////
//     Function: ThreadLinuxImpl::Destructor
//       Access: Public
//  Description: 
////////////////////////////////////////////////////////////////////
ThreadLinuxImpl::
~ThreadLinuxImpl() {
  if (thread_cat.is_debug()) {
    thread_cat.debug() << "Deleting thread " << _parent_obj->get_name() << "\n";
  }

  if (_stack != NULL) {
    delete[] _stack;
  }
}

////////////////////////////////////////////////////////////////////
//     Function: ThreadLinuxImpl::start
//       Access: Public
//  Description: 
////////////////////////////////////////////////////////////////////
bool ThreadLinuxImpl::
start(ThreadPriority priority, bool global, bool joinable) {
  _mutex.lock();
  if (thread_cat.is_debug()) {
    thread_cat.debug() << "Starting " << *_parent_obj << "\n";
  }

  nassertd(_status == S_new && _thread == 0 && _stack == NULL) {
    _mutex.release();
    return false;
  }

  _joinable = joinable;
  _status = S_start_called;

  if (!_got_main_thread_pointer) {
    // If we haven't spawned any threads yet, this must be the main
    // thread.
    bind_thread(Thread::get_main_thread());
    _got_main_thread_pointer = true;
  }

  // Increment the parent object's reference count first.  The thread
  // will eventually decrement it when it terminates.
  _parent_obj->ref();

  _stack = new unsigned char[thread_stack_size];

  int flags = SIGCHLD | CLONE_PARENT | CLONE_VM;
  if (!global) {
    // Make a thread that uses the same pid as the parent.
    flags |= CLONE_SIGHAND | CLONE_THREAD;
  }

  _thread = 
    clone(&root_func, _stack + thread_stack_size, 
          flags, (void *)this);

  if (_thread == -1) {
    // Oops, we couldn't start the thread.  Be sure to decrement the
    // reference count we incremented above, and return false to
    // indicate failure.
    perror("clone");
    unref_delete(_parent_obj);
    _mutex.release();
    return false;
  }

  _mutex.release();
  return true;
}

////////////////////////////////////////////////////////////////////
//     Function: ThreadLinuxImpl::interrupt
//       Access: Public
//  Description: Sends an interrupt message to the thread.  This will
//               interrupt any blocking-type system calls the thread
//               may be waiting on, such as I/O, so that the thread
//               may continue some other processing.  The specific
//               behavior is implementation dependent.
////////////////////////////////////////////////////////////////////
void ThreadLinuxImpl::
interrupt() {
}

////////////////////////////////////////////////////////////////////
//     Function: ThreadLinuxImpl::join
//       Access: Public
//  Description: Blocks the calling process until the thread
//               terminates.  If the thread has already terminated,
//               this returns immediately.
////////////////////////////////////////////////////////////////////
void ThreadLinuxImpl::
join() {
  _mutex.lock();
  nassertd(_joinable && _status != S_new) {
    _mutex.release();
    return;
  }

  while (_status != S_finished) {
    _cv.wait();
  }
  _mutex.release();
}

////////////////////////////////////////////////////////////////////
//     Function: ThreadLinuxImpl::get_current_thread
//       Access: Public, Static
//  Description: 
////////////////////////////////////////////////////////////////////
Thread *ThreadLinuxImpl::
get_current_thread() {
  if (!_got_main_thread_pointer) {
    // If we haven't spawned any threads yet, this must be the main
    // thread.
    bind_thread(Thread::get_main_thread());
    _got_main_thread_pointer = true;
    return Thread::get_main_thread();
  }

  _thread_pointers_lock.lock();
  Thread *result = NULL;

  ThreadPointers::const_iterator ti;
  pid_t my_tid = gettid();
  ti = _thread_pointers.find(my_tid);
  if (ti != _thread_pointers.end()) {
    result = (*ti).second;
  }
  _thread_pointers_lock.release();

  nassertr(result != NULL, NULL);
  return result;
}

////////////////////////////////////////////////////////////////////
//     Function: ThreadLinuxImpl::bind_thread
//       Access: Public, Static
//  Description: Associates the indicated Thread object with the
//               currently-executing thread.  You should not call this
//               directly; use Thread::bind_thread() instead.
////////////////////////////////////////////////////////////////////
void ThreadLinuxImpl::
bind_thread(Thread *thread) {
  _thread_pointers_lock.lock();
  pid_t my_tid = gettid();
  bool inserted = _thread_pointers.insert(ThreadPointers::value_type(my_tid, thread)).second;
  _thread_pointers_lock.release();
  nassertv(inserted);
}

////////////////////////////////////////////////////////////////////
//     Function: ThreadLinuxImpl::root_func
//       Access: Private, Static
//  Description: The entry point of each thread.
////////////////////////////////////////////////////////////////////
int ThreadLinuxImpl::
root_func(void *data) {
  ThreadLinuxImpl *self = (ThreadLinuxImpl *)data;

  {
    _thread_pointers_lock.lock();
    pid_t my_tid = gettid();

    bool inserted = _thread_pointers.insert(ThreadPointers::value_type(my_tid, self->_parent_obj)).second;
    nassertd(inserted) {
      _thread_pointers_lock.release();
      return 1;
    }
    _thread_pointers_lock.release();
  }

  {
    self->_mutex.lock();
    nassertd(self->_status == S_start_called) {
      self->_mutex.release();
      return 1;
    }
    
    self->_status = S_running;
    self->_cv.signal();
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
      return 1;
    }
    self->_status = S_finished;
    self->_cv.signal();
    self->_mutex.release();
  }

  // Now drop the parent object reference that we grabbed in start().
  // This might delete the parent object, and in turn, delete the
  // ThreadLinuxImpl object.
  unref_delete(self->_parent_obj);

  return 0;
}

#endif  // THREAD_LINUX_IMPL
