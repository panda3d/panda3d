// Filename: threadSimpleImpl.cxx
// Created by:  drose (18Jun07)
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

#include "threadSimpleImpl.h"
#include "threadSimpleManager.h"
#include "thread.h"

ThreadSimpleImpl *volatile ThreadSimpleImpl::_st_this;

////////////////////////////////////////////////////////////////////
//     Function: ThreadSimpleImpl::Constructor
//       Access: Public
//  Description: 
////////////////////////////////////////////////////////////////////
ThreadSimpleImpl::
ThreadSimpleImpl(Thread *parent_obj) :
  _parent_obj(parent_obj)
{
  _status = S_new;
  _joinable = false;
  _time_per_epoch = 0.0;
  _start_time = 0.0;

  _stack_size = (size_t)thread_stack_size;

  // We allocate the requested stack size, plus an additional tiny
  // buffer to allow room for the code to access values on the
  // currently executing stack frame at the time we switch the stack.
  _stack = (unsigned char *)malloc(_stack_size + 0x100);

  // Save this pointer for convenience.
  _manager = ThreadSimpleManager::get_global_ptr();
}

////////////////////////////////////////////////////////////////////
//     Function: ThreadSimpleImpl::Destructor
//       Access: Public
//  Description: 
////////////////////////////////////////////////////////////////////
ThreadSimpleImpl::
~ThreadSimpleImpl() {
  if (thread_cat.is_debug()) {
    thread_cat.debug() 
      << "Deleting thread " << _parent_obj->get_name() << "\n";
  }
  nassertv(_status != S_running);

  free(_stack);
}

////////////////////////////////////////////////////////////////////
//     Function: ThreadSimpleImpl::setup_main_thread
//       Access: Public
//  Description: Called for the main thread only, which has been
//               already started, to fill in the values appropriate to
//               that thread.
////////////////////////////////////////////////////////////////////
void ThreadSimpleImpl::
setup_main_thread() {
  _status = S_running;
  _time_per_epoch = 0.05;

  _manager->set_current_thread(this);
}

////////////////////////////////////////////////////////////////////
//     Function: ThreadSimpleImpl::start
//       Access: Public
//  Description: 
////////////////////////////////////////////////////////////////////
bool ThreadSimpleImpl::
start(ThreadPriority priority, bool joinable) {
  if (thread_cat.is_debug()) {
    thread_cat.debug() << "Starting " << *_parent_obj << "\n";
  }

  nassertr(_status == S_new, false);

  _joinable = joinable;
  _status = S_running;

  switch (priority) {
  case TP_low:
    _time_per_epoch = 0.02;
    break;
    
  case TP_normal:
    _time_per_epoch = 0.05;
    break;
    
  case TP_high:
    _time_per_epoch = 0.20;
    break;

  case TP_urgent:
    _time_per_epoch = 0.50;
    break;
  }

  // We'll keep the reference count upped while the thread is running.
  // When the thread finishes, we'll drop the reference count.
  _parent_obj->ref();

#ifdef HAVE_PYTHON
  // Query the current Python thread state.
  _python_state = PyThreadState_Swap(NULL);
  PyThreadState_Swap(_python_state);
#endif  // HAVE_PYTHON

  setup_context();

  _manager->enqueue_ready(this);
  return true;
}

////////////////////////////////////////////////////////////////////
//     Function: ThreadSimpleImpl::join
//       Access: Public
//  Description: Blocks the calling process until the thread
//               terminates.  If the thread has already terminated,
//               this returns immediately.
////////////////////////////////////////////////////////////////////
void ThreadSimpleImpl::
join() {
  nassertv(_joinable);
  if (_status == S_running) {
    ThreadSimpleImpl *thread = _manager->get_current_thread();
    _joining_threads.push_back(thread);
    _manager->next_context();
  }
}

////////////////////////////////////////////////////////////////////
//     Function: ThreadSimpleImpl::preempt
//       Access: Public
//  Description: 
////////////////////////////////////////////////////////////////////
void ThreadSimpleImpl::
preempt() {
  _manager->preempt(this);
}

////////////////////////////////////////////////////////////////////
//     Function: ThreadSimpleImpl::prepare_for_exit
//       Access: Public, Static
//  Description: Waits for all threads to terminate.  Normally this is
//               called only from the main thread.
////////////////////////////////////////////////////////////////////
void ThreadSimpleImpl::
prepare_for_exit() {
  ThreadSimpleManager *manager = ThreadSimpleManager::get_global_ptr();
  manager->prepare_for_exit();
}

////////////////////////////////////////////////////////////////////
//     Function: ThreadSimpleImpl::sleep_this
//       Access: Public
//  Description: 
////////////////////////////////////////////////////////////////////
void ThreadSimpleImpl::
sleep_this(double seconds) {
  _manager->enqueue_sleep(this, seconds);
  _manager->next_context();
}

////////////////////////////////////////////////////////////////////
//     Function: ThreadSimpleImpl::yield_this
//       Access: Public
//  Description: 
////////////////////////////////////////////////////////////////////
void ThreadSimpleImpl::
yield_this() {
  _manager->enqueue_ready(this);
  _manager->next_context();
}

////////////////////////////////////////////////////////////////////
//     Function: ThreadSimpleImpl::setup_context
//       Access: Private
//  Description: Fills the _context with an appropriate context buffer
//               and an appropriate stack reserved for the thread.
////////////////////////////////////////////////////////////////////
void ThreadSimpleImpl::
setup_context() {
  jmp_buf orig_stack;
  if (setjmp(orig_stack) == 0) {
    // First, switch to the new stack.  This requires temporarily saving
    // the this pointer as a static.
    _st_this = this;

    // Save the current context using setjmp().  This saves out all of
    // the processor register values, though it doesn't muck with the
    // stack.
    jmp_buf temp;
    if (setjmp(temp) == 0) {
      // This is the initial return from setjmp.  Still the original
      // stack.

      // Now we overwrite the stack pointer value in the saved
      // register context.  This doesn't work with all implementations
      // of setjmp/longjmp.
      ((void *&)temp[JB_SP]) = (_st_this->_stack + _st_this->_stack_size);

      // And finally, we place ourselves on the new stack by using
      // longjmp() to reload the saved (and modified) context.
      longjmp(temp, 1);
    }

    // This is the second return from setjmp.  Now we're on the new
    // stack.
    setup_context_2(_st_this);
    
    // Now restore the original stack and return.
    longjmp(orig_stack, 1);
  }

  // By now we are back to the original stack.
}

////////////////////////////////////////////////////////////////////
//     Function: ThreadSimpleImpl::setup_context_3
//       Access: Private
//  Description: More continuation of setup_context().  Again, making
//               this a separate function helps defeat the compiler
//               optimizer.
////////////////////////////////////////////////////////////////////
void ThreadSimpleImpl::
setup_context_3() {
#ifdef HAVE_PYTHON
  PyThreadState_Swap(_python_state);
#endif  // HAVE_PYTHON

  // Here we are executing within the thread.  Run the thread_main
  // function defined for this thread.
  _parent_obj->thread_main();
  
  // Now we have completed the thread.
  _status = S_finished;

  // Any threads that were waiting to join with this thread now become ready.
  JoiningThreads::iterator jti;
  for (jti = _joining_threads.begin(); jti != _joining_threads.end(); ++jti) {
    _manager->enqueue_ready(*jti);
  }
  _joining_threads.clear();

  _manager->enqueue_finished(this);
  _manager->next_context();
  
  // Shouldn't get here.
  abort();
}

#endif  // THREAD_SIMPLE_IMPL
