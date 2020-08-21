/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file thread.cxx
 * @author drose
 * @date 2002-08-08
 */

#include "thread.h"
#include "mainThread.h"
#include "externalThread.h"
#include "config_pipeline.h"
#include "mutexDebug.h"
#include "conditionVarDebug.h"

Thread *Thread::_main_thread;
Thread *Thread::_external_thread;
TypeHandle Thread::_type_handle;

/**
 * Creates a new Thread object, but does not immediately start executing it.
 * This gives the caller a chance to store it in a PT(Thread) object, if
 * desired, before the thread gets a chance to terminate and destruct itself.
 *
 * Call start() to begin thread execution.
 *
 * The name should be unique for each thread (though this is not enforced, and
 * not strictly required). The sync_name can be shared between multiple
 * different threads; threads that run synchronously with each other should be
 * given the same sync_name, for the benefit of PStats.
 */
Thread::
Thread(const std::string &name, const std::string &sync_name) :
  Namable(name),
  _sync_name(sync_name),
  _impl(this)
{
  _started = false;
  _pstats_index = -1;
  _python_index = -1;
  _pstats_callback = nullptr;
  _pipeline_stage = 0;
  _joinable = false;
  _current_task = nullptr;

#ifdef DEBUG_THREADS
  _blocked_on_mutex = nullptr;
  _waiting_on_cvar = nullptr;
#endif
}

/**
 *
 */
Thread::
~Thread() {
#ifdef DEBUG_THREADS
  nassertv(_blocked_on_mutex == nullptr &&
           _waiting_on_cvar == nullptr);
#endif
}

/**
 * Returns a new Panda Thread object associated with the current thread (which
 * has been created externally). This can be used to bind a unique Panda
 * Thread object with an external thread, such as a new Python thread.
 *
 * It is particularly useful to bind a Panda Thread object to an external
 * thread for the purposes of PStats monitoring.  Without this call, each
 * external thread will be assigned the same global ExternalThread object,
 * which means they will all appear in the same PStats graph.
 *
 * It is the caller's responsibility to save the returned Thread pointer for
 * the lifetime of the external thread.  It is an error for the Thread pointer
 * to destruct while the external thread is still in the system.
 *
 * It is also an error to call this method from the main thread, or twice
 * within a given thread, unless it is given the same name each time (in which
 * case the same pointer will be returned each time).
 */
PT(Thread) Thread::
bind_thread(const std::string &name, const std::string &sync_name) {
  Thread *current_thread = get_current_thread();
  if (current_thread != get_external_thread()) {
    // This thread already has an associated thread.
    nassertr(current_thread->get_name() == name &&
             current_thread->get_sync_name() == sync_name, current_thread);
    return current_thread;
  }

  PT(Thread) thread = new ExternalThread(name, sync_name);
  ThreadImpl::bind_thread(thread);
  return thread;
}

/**
 * Specifies the Pipeline stage number associated with this thread.  The
 * default stage is 0 if no stage is specified otherwise.
 *
 * This must be a value in the range [0 .. pipeline->get_num_stages() - 1].
 * It specifies the values that this thread observes for all pipelined data.
 * Typically, an application thread will leave this at 0, but a render thread
 * may set it to 1 or 2 (to operate on the previous frame's data, or the
 * second previous frame's data).
 */
void Thread::
set_pipeline_stage(int pipeline_stage) {
#ifdef THREADED_PIPELINE
  _pipeline_stage = pipeline_stage;
#else
  if (pipeline_stage != 0) {
    pipeline_cat.warning()
      << "Requested pipeline stage " << pipeline_stage
      << " but multithreaded render pipelines not enabled in build.\n";
  }
  _pipeline_stage = 0;
#endif
}

/**
 *
 */
void Thread::
output(std::ostream &out) const {
  out << get_type() << " " << get_name();
}

/**
 * Writes a description of the mutex or condition variable that this thread is
 * blocked on.  Writes nothing if there is no blocker, or if we are not in
 * DEBUG_THREADS mode.
 */
void Thread::
output_blocker(std::ostream &out) const {
#ifdef DEBUG_THREADS
  if (_blocked_on_mutex != nullptr) {
    _blocked_on_mutex->output_with_holder(out);
  } else if (_waiting_on_cvar != nullptr) {
    out << *_waiting_on_cvar;
  }
#endif  // DEBUG_THREADS
}

/**
 *
 */
void Thread::
write_status(std::ostream &out) {
#if defined(HAVE_THREADS) && defined(SIMPLE_THREADS)
  ThreadImpl::write_status(out);
#endif
}

/**
 * Starts the thread executing.  It is only valid to call this once.
 *
 * The thread will begin executing its thread_main() function, and will
 * terminate when thread_main() returns.
 *
 * priority is intended as a hint to the relative importance of this thread.
 * This may be ignored by the thread implementation.
 *
 * joinable should be set true if you intend to call join() to wait for the
 * thread to terminate, or false if you don't care and you will never call
 * join(). Note that the reference count on the Thread object is incremented
 * while the thread itself is running, so if you just want to fire and forget
 * a thread, you may pass joinable = false, and never store the Thread object.
 * It will automatically destruct itself when it finishes.
 *
 * The return value is true if the thread is successfully started, false
 * otherwise.
 */
bool Thread::
start(ThreadPriority priority, bool joinable) {
  nassertr(!_started, false);

  if (!support_threads) {
    thread_cat->warning()
      << *this << " could not be started: support-threads is false.\n";
    return false;
  }

  _joinable = joinable;
  _started = _impl.start(priority, joinable);

  if (!_started) {
    thread_cat->warning()
      << *this << " could not be started!\n";
  }

  return _started;
}

/**
 * Creates the Thread object that represents the main thread.
 */
void Thread::
init_main_thread() {
  // There is a chance of mutual recursion at startup.  The count variable
  // here attempts to protect against that.
  static int count = 0;
  ++count;
  if (count == 1 && _main_thread == nullptr) {
    init_memory_hook();
    _main_thread = new MainThread;
    _main_thread->ref();
  }
}

/**
 * Creates the Thread object that represents all of the external threads.
 */
void Thread::
init_external_thread() {
  if (_external_thread == nullptr) {
    init_memory_hook();
    _external_thread = new ExternalThread;
    _external_thread->ref();
  }
}

/**
 * Since this class is just an interface definition, there is no need to have
 * a destructor.  However, we must have one anyway to stop gcc's annoying
 * warning.
 */
Thread::PStatsCallback::
~PStatsCallback() {
}

/**
 * Called when the thread is deactivated (swapped for another running thread).
 * This is intended to provide a callback hook for PStats to assign time to
 * individual threads properly, particularly in the SIMPLE_THREADS case.
 */
void Thread::PStatsCallback::
deactivate_hook(Thread *) {
}

/**
 * Called when the thread is activated (resumes execution).  This is intended
 * to provide a callback hook for PStats to assign time to individual threads
 * properly, particularly in the SIMPLE_THREADS case.
 */
void Thread::PStatsCallback::
activate_hook(Thread *) {
}
