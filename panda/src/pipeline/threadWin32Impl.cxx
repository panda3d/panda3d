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

#include <windows.h>

static thread_local Thread *_current_thread = nullptr;
static patomic_flag _main_thread_known = ATOMIC_FLAG_INIT;

#ifndef CREATE_WAITABLE_TIMER_HIGH_RESOLUTION
#define CREATE_WAITABLE_TIMER_HIGH_RESOLUTION 0x00000002
#endif

#if _WIN32_WINNT < 0x0601
// Requires Windows 7.
static DWORD (__stdcall *EnableThreadProfiling)(HANDLE, DWORD, DWORD64, HANDLE *) = nullptr;
static DWORD (__stdcall *DisableThreadProfiling)(HANDLE) = nullptr;
static DWORD (__stdcall *ReadThreadProfilingData)(HANDLE, DWORD, PPERFORMANCE_DATA data) = nullptr;

static bool init_thread_profiling() {
  static bool inited = false;
  if (!inited) {
    HMODULE kernel32 = GetModuleHandleA("kernel32.dll");
    EnableThreadProfiling = (decltype(EnableThreadProfiling))GetProcAddress(kernel32, "EnableThreadProfiling");
    DisableThreadProfiling = (decltype(DisableThreadProfiling))GetProcAddress(kernel32, "DisableThreadProfiling");
    ReadThreadProfilingData = (decltype(ReadThreadProfilingData))GetProcAddress(kernel32, "ReadThreadProfilingData");
    inited = true;
  }
  return (EnableThreadProfiling && DisableThreadProfiling && ReadThreadProfilingData);
}
#else
static bool init_thread_profiling() {
  return true;
}
#endif

/**
 * Called by get_current_thread() if the current thread pointer is null; checks
 * whether it might be the main thread.
 * Note that adding noinline speeds up this call *significantly*, don't remove!
 */
static __declspec(noinline) Thread *
init_current_thread() {
  Thread *thread = _current_thread;
  if (!_main_thread_known.test_and_set(std::memory_order_relaxed)) {
    // Assume that we must be in the main thread, since this method must be
    // called before the first thread is spawned.
    thread = Thread::get_main_thread();
    _current_thread = thread;
  }
  // If this assertion triggers, you are making Panda calls from a thread
  // that has not first been registered using Thread::bind_thread().
  nassertr(thread != nullptr, nullptr);
  return thread;
}

/**
 *
 */
ThreadWin32Impl::
~ThreadWin32Impl() {
  if (thread_cat->is_debug()) {
    thread_cat.debug() << "Deleting thread " << _parent_obj->get_name() << "\n";
  }

  CloseHandle(_thread);

  if (_timer != nullptr) {
    CloseHandle(_timer);
    _timer = nullptr;
  }
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
 *
 */
Thread *ThreadWin32Impl::
get_current_thread() {
  Thread *thread = _current_thread;
  return (thread != nullptr) ? thread : init_current_thread();
}

/**
 * Associates the indicated Thread object with the currently-executing thread,
 * unless a thread is already bound, in which case it is returned.
 * You should not call this directly; use Thread::bind_thread() instead.
 */
Thread *ThreadWin32Impl::
bind_thread(Thread *thread) {
  if (_current_thread != nullptr) {
    return _current_thread;
  }
  if (thread == Thread::get_main_thread()) {
    _main_thread_known.test_and_set(std::memory_order_relaxed);
  }
  _current_thread = thread;
  return thread;
}


/**
 *
 */
void ThreadWin32Impl::
sleep(double seconds) {
  Thread *thread = get_current_thread();
  ThreadWin32Impl *self = &thread->_impl;

  HANDLE timer = self->_timer;
  if (timer == nullptr) {
    timer = CreateWaitableTimerExW(nullptr, nullptr, CREATE_WAITABLE_TIMER_MANUAL_RESET | CREATE_WAITABLE_TIMER_HIGH_RESOLUTION, TIMER_ALL_ACCESS);
    self->_timer = timer;
  }

  LARGE_INTEGER ft;
  ft.QuadPart = seconds * -10000000LL;
  SetWaitableTimer(timer, &ft, 0, nullptr, nullptr, 0);
  WaitForSingleObject(timer, INFINITE);
}

/**
 * Returns the number of context switches that occurred on the current thread.
 * The first number is the total number of context switches reported by the OS,
 * and the second number is the number of involuntary context switches (ie. the
 * thread was scheduled out by the OS), if known, otherwise zero.
 * Returns true if context switch information was available, false otherwise.
 */
bool ThreadWin32Impl::
get_context_switches(size_t &total, size_t &involuntary) {
  Thread *thread = get_current_thread();
  ThreadWin32Impl *self = &thread->_impl;

  if (!self->_profiling && init_thread_profiling()) {
    DWORD result = EnableThreadProfiling(GetCurrentThread(), THREAD_PROFILING_FLAG_DISPATCH, 0, &self->_profiling);
    if (result != ERROR_SUCCESS) {
      self->_profiling = 0;
      return false;
    }
  }

  PERFORMANCE_DATA data = {sizeof(PERFORMANCE_DATA), PERFORMANCE_DATA_VERSION};
  if (ReadThreadProfilingData(self->_profiling, READ_THREAD_PROFILING_FLAG_DISPATCHING, &data) == ERROR_SUCCESS) {
    total = data.ContextSwitchCount;
    involuntary = 0;
    return true;
  }
  return false;
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
    _current_thread = self->_parent_obj;

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

    if (self->_profiling != 0) {
      DisableThreadProfiling(self->_profiling);
      self->_profiling = 0;
    }

    // Now drop the parent object reference that we grabbed in start(). This
    // might delete the parent object, and in turn, delete the ThreadWin32Impl
    // object.
    unref_delete(self->_parent_obj);
  }

  return 0;
}

#endif  // THREAD_WIN32_IMPL
