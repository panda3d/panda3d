// Filename: threadNsprImpl.cxx
// Created by:  drose (08Aug02)
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

#include "threadNsprImpl.h"
#include "selectThreadImpl.h"

#ifdef THREAD_NSPR_IMPL

#include "pointerTo.h"
#include "config_pipeline.h"

PRUintn ThreadNsprImpl::_pt_ptr_index = 0;
bool ThreadNsprImpl::_got_pt_ptr_index = false;

////////////////////////////////////////////////////////////////////
//     Function: ThreadNsprImpl::Destructor
//       Access: Public
//  Description: 
////////////////////////////////////////////////////////////////////
ThreadNsprImpl::
~ThreadNsprImpl() {
  if (thread_cat.is_debug()) {
    thread_cat.debug() << "Deleting thread " << _parent_obj->get_name() << "\n";
  }

  // If the thread object is destructing, it means the last pointer
  // has gone away.  This can happen in one of three cases:

  // (1) start() was never called, and the last user reference has
  // gone away.  In this case, we should do nothing.

  // (2) There are no more user references, and the thread is still
  // running but is about to exit root_func().  In this case, no one
  // will ever call join().

  // (3) The thread terminated a while ago, and the last user
  // reference has just gone away.  In this case, we should call
  // join() if no one else has, to clean up whatever internal
  // structures NSPR might keep around for a non-detached thread.


  if (_thread == (PRThread *)NULL) {
    // Since the _thread pointer is still NULL, we must either be in
    // case (1), or case (3) but someone else has already called
    // join().  Do nothing.
    return;
  }

  PRThread *current_thread = PR_GetCurrentThread();
  if (current_thread == _thread) {
    // Since we are currently executing *this* thread, we must be in
    // case (2).  Unfortunately, we cannot now indicate we need to
    // clean up the thread, since NSPR doesn't have an interface to
    // make a thread unjoinable after it has been created, and a
    // thread can't join itself (can it?).
    if (_joinable) {
      thread_cat.warning()
        << "thread " << _parent_obj->get_name() << " was never joined.\n";
    }
    return;
  }

  // This is case (3).  The pointer went away and the thread has
  // already terminated; furthermore, no one has called join() yet.
  // We should do it.
  join();
}

////////////////////////////////////////////////////////////////////
//     Function: ThreadNsprImpl::start
//       Access: Public
//  Description: 
////////////////////////////////////////////////////////////////////
bool ThreadNsprImpl::
start(ThreadPriority priority, bool global, bool joinable) {
  _mutex.lock();
  if (thread_cat.is_debug()) {
    thread_cat.debug() << "Starting " << *_parent_obj << "\n";
  }
  nassertd(_thread == (PRThread *)NULL) {
    _mutex.release();
    return false;
  }

  _joinable = joinable;

  if (!_got_pt_ptr_index) {
    init_pt_ptr_index();
  }

  PRThreadPriority nspr_pri;

  switch (priority) {
  case TP_low:
    nspr_pri = PR_PRIORITY_LOW;
    break;

  case TP_high:
    nspr_pri = PR_PRIORITY_HIGH;
    break;

  case TP_urgent:
    nspr_pri = PR_PRIORITY_URGENT;
    break;

  case TP_normal:
  default:
    nspr_pri = PR_PRIORITY_NORMAL;
  }

  PRThreadScope nspr_scope = (global) ? PR_GLOBAL_THREAD : PR_LOCAL_THREAD;

  PRThreadState nspr_state = (_joinable) ? PR_JOINABLE_THREAD : PR_UNJOINABLE_THREAD;

  // Increment the parent object's reference count first.  The thread
  // will eventually decrement it when it terminates.
  _parent_obj->ref();
  _thread = 
    PR_CreateThread(PR_USER_THREAD, &root_func, (void *)_parent_obj,
                    nspr_pri, nspr_scope, nspr_state, 0);

  if (_thread == (PRThread *)NULL) {
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
//     Function: ThreadNsprImpl::interrupt
//       Access: Public
//  Description: Sends an interrupt message to the thread.  This will
//               interrupt any blocking-type system calls the thread
//               may be waiting on, such as I/O, so that the thread
//               may continue some other processing.  The specific
//               behavior is implementation dependent.
////////////////////////////////////////////////////////////////////
void ThreadNsprImpl::
interrupt() {
  if (_thread != (PRThread *)NULL) {
    PR_Interrupt(_thread);
  }
}

////////////////////////////////////////////////////////////////////
//     Function: ThreadNsprImpl::join
//       Access: Public
//  Description: Blocks the calling process until the thread
//               terminates.  If the thread has already terminated,
//               this returns immediately.
////////////////////////////////////////////////////////////////////
void ThreadNsprImpl::
join() {
  _mutex.lock();
  if (_joinable && _thread != (PRThread *)NULL) {
    PR_JoinThread(_thread);
    _thread = (PRThread *)NULL;
  }
  _mutex.release();
}

////////////////////////////////////////////////////////////////////
//     Function: ThreadNsprImpl::root_func
//       Access: Private, Static
//  Description: The entry point of each thread.
////////////////////////////////////////////////////////////////////
void ThreadNsprImpl::
root_func(void *data) {
  Thread *parent_obj = (Thread *)data;
  PRStatus result = PR_SetThreadPrivate(_pt_ptr_index, parent_obj);
  nassertv(result == PR_SUCCESS);
  parent_obj->thread_main();

  if (thread_cat.is_debug()) {
    thread_cat.debug()
      << "Terminating thread " << parent_obj->get_name() 
      << ", count = " << parent_obj->get_ref_count() << "\n";
  }

  // Now drop the parent object reference that we grabbed in start().
  // This might delete the parent object, and in turn, delete the
  // ThreadNsprImpl object.
  unref_delete(parent_obj);
}

////////////////////////////////////////////////////////////////////
//     Function: ThreadNsprImpl::init_pt_ptr_index
//       Access: Private, Static
//  Description: Allocate a new index to store the Thread parent
//               pointer as a piece of per-thread private data.
////////////////////////////////////////////////////////////////////
void ThreadNsprImpl::
init_pt_ptr_index() {
  nassertv(!_got_pt_ptr_index);

  PRStatus result = PR_NewThreadPrivateIndex(&_pt_ptr_index, NULL);
  if (result != PR_SUCCESS) {
    thread_cat.error()
      << "Unable to associate Thread pointers with threads.\n";
    return;
  }

  _got_pt_ptr_index = true;

  // Assume that we must be in the main thread, since this method must
  // be called before the first thread is spawned.
  Thread *main_thread_obj = Thread::get_main_thread();
  result = PR_SetThreadPrivate(_pt_ptr_index, main_thread_obj);
  nassertv(result == PR_SUCCESS);
}

#endif  // THREAD_NSPR_IMPL
