/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file threadWin32Impl.cxx
 * @author drose
 * @date 2006-02-07
 */

#include "threadWin32Impl.h"
#include "selectThreadImpl.h"

#ifdef THREAD_WIN32_IMPL

#include "thread.h"
#include "pointerTo.h"
#include "config_pipeline.h"

DWORD ThreadWin32Impl::_pt_ptr_index = 0;
bool ThreadWin32Impl::_got_pt_ptr_index = false;

/**
 *
 */
ThreadWin32Impl::
~ThreadWin32Impl() {
  if (thread_cat->is_debug()) {
    thread_cat.debug() << "Deleting thread " << _parent_obj->get_name() << "\n";
  }

  CloseHandle(_thread);
}

/**
 * Called for the main thread only, which has been already started, to fill in
 * the values appropriate to that thread.
 */
void ThreadWin32Impl::
setup_main_thread() {
  _status = S_running;
}

/**
 *
 */
bool ThreadWin32Impl::
start(ThreadPriority priority, bool joinable) {
  _mutex.lock();
  if (thread_cat->is_debug()) {
    thread_cat.debug() << "Starting " << *_parent_obj << "\n";
  }

  nassertd(_status == S_new && _thread == 0) {
    _mutex.unlock();
    return false;
  }

  _joinable = joinable;
  _status = S_start_called;

  if (!_got_pt_ptr_index) {
    init_pt_ptr_index();
  }

  // Increment the parent object's reference count first.  The thread will
  // eventually decrement it when it terminates.
  _parent_obj->ref();
  _thread =
    CreateThread(nullptr, 0, &root_func, (void *)this, 0, &_thread_id);

  if (_thread_id == 0) {
    // Oops, we couldn't start the thread.  Be sure to decrement the reference
    // count we incremented above, and return false to indicate failure.
    unref_delete(_parent_obj);
    _mutex.unlock();
    return false;
  }

  // Thread was successfully started.  Set the priority as specified.
  switch (priority) {
  case TP_low:
    SetThreadPriority(_thread, THREAD_PRIORITY_BELOW_NORMAL);
    break;

  case TP_high:
    SetThreadPriority(_thread, THREAD_PRIORITY_ABOVE_NORMAL);
    break;

  case TP_urgent:
    SetThreadPriority(_thread, THREAD_PRIORITY_HIGHEST);
    break;

  case TP_normal:
  default:
    SetThreadPriority(_thread, THREAD_PRIORITY_NORMAL);
    break;
  }

  _mutex.unlock();
  return true;
}

/**
 * Blocks the calling process until the thread terminates.  If the thread has
 * already terminated, this returns immediately.
 */
void ThreadWin32Impl::
join() {
  _mutex.lock();
  nassertd(_joinable && _status != S_new) {
    _mutex.unlock();
    return;
  }

  while (_status != S_finished) {
    _cv.wait();
  }
  _mutex.unlock();
}

/**
 *
 */
std::string ThreadWin32Impl::
get_unique_id() const {
  std::ostringstream strm;
  strm << GetCurrentProcessId() << "." << _thread_id;

  return strm.str();
}

/**
 * The entry point of each thread.
 */
DWORD ThreadWin32Impl::
root_func(LPVOID data) {
  TAU_REGISTER_THREAD();
  {
    // TAU_PROFILE("void ThreadWin32Impl::root_func()", " ", TAU_USER);

    ThreadWin32Impl *self = (ThreadWin32Impl *)data;
    BOOL result = TlsSetValue(_pt_ptr_index, self->_parent_obj);
    nassertr(result, 1);

    {
      self->_mutex.lock();
      nassertd(self->_status == S_start_called) {
        self->_mutex.unlock();
        return 1;
      }
      self->_status = S_running;
      self->_cv.notify();
      self->_mutex.unlock();
    }

    self->_parent_obj->thread_main();

    if (thread_cat->is_debug()) {
      thread_cat.debug()
        << "Terminating thread " << self->_parent_obj->get_name()
        << ", count = " << self->_parent_obj->get_ref_count() << "\n";
    }

    {
      self->_mutex.lock();
      nassertd(self->_status == S_running) {
        self->_mutex.unlock();
        return 1;
      }
      self->_status = S_finished;
      self->_cv.notify();
      self->_mutex.unlock();
    }

    // Now drop the parent object reference that we grabbed in start(). This
    // might delete the parent object, and in turn, delete the ThreadWin32Impl
    // object.
    unref_delete(self->_parent_obj);
  }

  return 0;
}

/**
 * Allocate a new index to store the Thread parent pointer as a piece of per-
 * thread private data.
 */
void ThreadWin32Impl::
init_pt_ptr_index() {
  nassertv(!_got_pt_ptr_index);

  _pt_ptr_index = TlsAlloc();
  if (_pt_ptr_index == TLS_OUT_OF_INDEXES) {
    thread_cat->error()
      << "Unable to associate Thread pointers with threads.\n";
    return;
  }

  _got_pt_ptr_index = true;

  // Assume that we must be in the main thread, since this method must be
  // called before the first thread is spawned.
  Thread *main_thread_obj = Thread::get_main_thread();
  BOOL result = TlsSetValue(_pt_ptr_index, main_thread_obj);
  nassertv(result);
}

#endif  // THREAD_WIN32_IMPL
