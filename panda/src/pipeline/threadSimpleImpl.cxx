/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file threadSimpleImpl.cxx
 * @author drose
 * @date 2007-06-18
 */

#include "selectThreadImpl.h"

#ifdef THREAD_SIMPLE_IMPL

#include "threadSimpleImpl.h"
#include "threadSimpleManager.h"
#include "thread.h"

ThreadSimpleImpl *volatile ThreadSimpleImpl::_st_this;

int ThreadSimpleImpl::_next_unique_id = 1;

/**
 *
 */
ThreadSimpleImpl::
ThreadSimpleImpl(Thread *parent_obj) :
  _parent_obj(parent_obj)
{
  _unique_id = _next_unique_id;
  ++_next_unique_id;

  _status = TS_new;
  _joinable = false;
  _priority = TP_normal;
  _priority_weight = 1.0;
  _run_ticks = 0;
  _start_time = 0.0;
  _stop_time = 0.0;
  _wake_time = 0.0;

  _context = alloc_thread_context();
  _stack = nullptr;
  _stack_size = 0;

  // Save this pointer for convenience.
  _manager = ThreadSimpleManager::get_global_ptr();

#ifdef HAVE_POSIX_THREADS
  _posix_system_thread_id = (pthread_t)-1;
#endif
#ifdef WIN32
  _win32_system_thread_id = 0;
#endif
}

/**
 *
 */
ThreadSimpleImpl::
~ThreadSimpleImpl() {
  if (thread_cat->is_debug()) {
    thread_cat.debug()
      << "Deleting thread " << _parent_obj->get_name() << "\n";
  }
  nassertv(_status != TS_running);

  free_thread_context(_context);

  if (_stack != nullptr) {
    memory_hook->mmap_free(_stack, _stack_size);
  }
  _manager->remove_thread(this);
}

/**
 * Called for the main thread only, which has been already started, to fill in
 * the values appropriate to that thread.
 */
void ThreadSimpleImpl::
setup_main_thread() {
  _status = TS_running;
  _priority = TP_normal;
  _priority_weight = _manager->_simple_thread_normal_weight;

#ifdef HAVE_POSIX_THREADS
  _posix_system_thread_id = pthread_self();
#endif
#ifdef WIN32
  _win32_system_thread_id = GetCurrentThreadId();
#endif

  _manager->set_current_thread(this);
}

/**
 *
 */
bool ThreadSimpleImpl::
start(ThreadPriority priority, bool joinable) {
  if (thread_cat->is_debug()) {
    thread_cat.debug() << "Starting " << *_parent_obj << "\n";
  }

  nassertr(_status == TS_new, false);

  nassertr(_stack == nullptr, false);
  _stack_size = memory_hook->round_up_to_page_size((size_t)thread_stack_size);
  if (needs_stack_prealloc) {
    _stack = (unsigned char *)memory_hook->mmap_alloc(_stack_size, true);
  }

  _joinable = joinable;
  _status = TS_running;
  _priority = priority;

  switch (priority) {
  case TP_low:
    _priority_weight = _manager->_simple_thread_low_weight;
    break;

  case TP_normal:
    _priority_weight = _manager->_simple_thread_normal_weight;
    break;

  case TP_high:
    _priority_weight = _manager->_simple_thread_high_weight;
    break;

  case TP_urgent:
    _priority_weight = _manager->_simple_thread_urgent_weight;
    break;
  }

  // We'll keep the reference count upped while the thread is running.  When
  // the thread finishes, we'll drop the reference count.
  _parent_obj->ref();

#ifdef HAVE_PYTHON
  // Query the current Python thread state.
  _python_state = thread_state_swap(nullptr);
  thread_state_swap(_python_state);
#endif  // HAVE_PYTHON

  init_thread_context(_context, _stack, _stack_size, st_begin_thread, this);

  _manager->enqueue_ready(this, false);
  return true;
}

/**
 * Blocks the calling process until the thread terminates.  If the thread has
 * already terminated, this returns immediately.
 */
void ThreadSimpleImpl::
join() {
  nassertv(_joinable);
  if (_status == TS_running) {
    ThreadSimpleImpl *thread = _manager->get_current_thread();
    if (thread != this) {
      _joining_threads.push_back(thread);
      _manager->next_context();
    }
  }
}

/**
 *
 */
void ThreadSimpleImpl::
preempt() {
  _manager->preempt(this);
}

/**
 *
 */
std::string ThreadSimpleImpl::
get_unique_id() const {
  std::ostringstream strm;
#ifdef WIN32
  strm << GetCurrentProcessId();
#else
  strm << getpid();
#endif
  strm << "." << _unique_id;

  return strm.str();
}

/**
 * Waits for all threads to terminate.  Normally this is called only from the
 * main thread.
 */
void ThreadSimpleImpl::
prepare_for_exit() {
  ThreadSimpleManager *manager = ThreadSimpleManager::get_global_ptr();
  manager->prepare_for_exit();
}

/**
 *
 */
bool ThreadSimpleImpl::
is_true_threads() {
  return (is_os_threads != 0);
}

/**
 *
 */
void ThreadSimpleImpl::
sleep_this(double seconds) {
  _manager->enqueue_sleep(this, seconds);
  _manager->next_context();
}

/**
 *
 */
void ThreadSimpleImpl::
yield_this(bool volunteer) {
  if (thread_cat->is_debug() && volunteer) {
    thread_cat.debug()
      << "Force-yielding " << _parent_obj->get_name() << "\n";
  }
  _manager->enqueue_ready(this, true);
  _manager->next_context();
}

/**
 * This method is called as the first introduction to a new thread.
 */
void ThreadSimpleImpl::
st_begin_thread(void *data) {
  ThreadSimpleImpl *self = (ThreadSimpleImpl *)data;
  self->begin_thread();
}

/**
 * This method is called as the first introduction to a new thread.
 */
void ThreadSimpleImpl::
begin_thread() {
#ifdef HAVE_PYTHON
  thread_state_swap(_python_state);
#endif  // HAVE_PYTHON

#ifdef HAVE_POSIX_THREADS
  _posix_system_thread_id = pthread_self();
#endif
#ifdef WIN32
  _win32_system_thread_id = GetCurrentThreadId();
#endif

  // Here we are executing within the thread.  Run the thread_main function
  // defined for this thread.
  _parent_obj->thread_main();

  // Now we have completed the thread.
  _status = TS_finished;

  // Any threads that were waiting to join with this thread now become ready.
  JoiningThreads::iterator jti;
  for (jti = _joining_threads.begin(); jti != _joining_threads.end(); ++jti) {
    _manager->enqueue_ready(*jti, false);
  }
  _joining_threads.clear();

  _manager->enqueue_finished(this);
  _manager->next_context();

  // Shouldn't get here.
  nassertv(false);
  abort();
}

#endif  // THREAD_SIMPLE_IMPL
