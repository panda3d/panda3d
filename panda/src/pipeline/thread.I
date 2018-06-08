/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file thread.I
 * @author drose
 * @date 2002-08-08
 */

/**
 * Returns the sync name of the thread.  This name collects threads into "sync
 * groups", which are expected to run synchronously.  This is mainly used for
 * the benefit of PStats; threads with the same sync name can be ticked all at
 * once via the thread_tick() call.
 */
INLINE const std::string &Thread::
get_sync_name() const {
  return _sync_name;
}

/**
 * Returns the PStats index associated with this thread, or -1 if no index has
 * yet been associated with this thread.  This is used internally by the
 * PStatClient; you should not need to call this directly.
 */
INLINE int Thread::
get_pstats_index() const {
  return _pstats_index;
}

/**
 * Returns the Python index associated with this thread, or -1 if no index has
 * yet been associated with this thread.  This is used internally by the
 * direct.stdpy.thread module; you should not need to call this directly.
 */
INLINE int Thread::
get_python_index() const {
  return _python_index;
}

/**
 * Returns a string that is guaranteed to be unique to this thread, across all
 * processes on the machine, during at least the lifetime of this process.
 */
INLINE std::string Thread::
get_unique_id() const {
  return _impl.get_unique_id();
}

/**
 * Returns the Pipeline stage number associated with this thread.  The default
 * stage is 0 if no stage is specified otherwise.  See set_pipeline_stage().
 */
INLINE int Thread::
get_pipeline_stage() const {
#if !defined(_DEBUG) && defined(__has_builtin) && __has_builtin(__builtin_assume)
  // Because this is a signed int, this results in a sign extend on x86-64.
  // However, since we guarantee that this is never less than zero, clang
  // offers a nice way to avoid that.
  int pipeline_stage = _pipeline_stage;
  __builtin_assume(pipeline_stage >= 0);
  return pipeline_stage;
#else
  return _pipeline_stage;
#endif
}

/**
 * Sets this thread's pipeline stage number to at least the indicated value,
 * unless it is already larger.  See set_pipeline_stage().
 */
INLINE void Thread::
set_min_pipeline_stage(int min_pipeline_stage) {
  set_pipeline_stage(std::max(_pipeline_stage, min_pipeline_stage));
}

/**
 * Returns a pointer to the "main" Thread object--this is the Thread that
 * started the whole process.
 */
INLINE Thread *Thread::
get_main_thread() {
  if (_main_thread == nullptr) {
    init_main_thread();
  }
  return _main_thread;
}

/**
 * Returns a pointer to the "external" Thread object--this is a special Thread
 * object that corresponds to any thread spawned outside of Panda's threading
 * interface.  Note that multiple different threads may share this same
 * pointer.
 */
INLINE Thread *Thread::
get_external_thread() {
  if (_external_thread == nullptr) {
    init_external_thread();
  }
  return _external_thread;
}

/**
 * Returns a pointer to the currently-executing Thread object.  If this is
 * called from the main thread, this will return the same value as
 * get_main_thread().
 *
 * This will always return some valid Thread pointer.  It will never return
 * NULL, even if the current thread was spawned outside of Panda's threading
 * system, although all non-Panda threads will return the exact same Thread
 * pointer.
 */
INLINE Thread *Thread::
get_current_thread() {
  TAU_PROFILE("Thread *Thread::get_current_thread()", " ", TAU_USER);
#ifndef HAVE_THREADS
  return get_main_thread();
#else  // HAVE_THREADS
  Thread *thread = ThreadImpl::get_current_thread();
  if (thread == nullptr) {
    return Thread::get_external_thread();
  }
  return thread;
#endif  // HAVE_THREADS
}

/**
 * Returns the integer pipeline stage associated with the current thread.
 * This is the same thing as get_current_thread()->get_pipeline_stage(), but
 * it may be faster to retrieve in some contexts.
 */
INLINE int Thread::
get_current_pipeline_stage() {
  TAU_PROFILE("int Thread::get_current_pipeline_stage()", " ", TAU_USER);
#ifndef THREADED_PIPELINE
  // Without threaded pipelining, the result is always 0.
  return 0;
#else
  return get_current_thread()->get_pipeline_stage();
#endif  // !THREADED_PIPELINE
}

/**
 * Returns true if threading support has been compiled in and enabled, or
 * false if no threading is available (and Thread::start() will always fail).
 */
INLINE bool Thread::
is_threading_supported() {
  if (!support_threads) {
    return false;
  }
  return ThreadImpl::is_threading_supported();
}

/**
 * Returns true if a real threading library is available that supports actual
 * OS-implemented threads, or false if the only threading we can provide is
 * simulated user-space threading.
 */
INLINE bool Thread::
is_true_threads() {
  if (!support_threads) {
    return false;
  }
  return ThreadImpl::is_true_threads();
}

/**
 * Returns true if Panda is currently compiled for "simple threads", which is
 * to say, cooperative context switching only, reducing the need for quite so
 * many critical section protections.  This is not necessarily the opposite of
 * "true threads", since one possible implementation of simple threads is via
 * true threads with mutex protection to ensure only one runs at a time.
 */
INLINE bool Thread::
is_simple_threads() {
  if (!support_threads) {
    return false;
  }
  return ThreadImpl::is_simple_threads();
}

/**
 * Suspends the current thread for at least the indicated amount of time.  It
 * might be suspended for longer.
 */
INLINE void Thread::
sleep(double seconds) {
  TAU_PROFILE("void Thread::sleep(double)", " ", TAU_USER);
  ThreadImpl::sleep(seconds);
}

/**
 * Suspends the current thread for the rest of the current epoch.
 */
INLINE void Thread::
force_yield() {
  TAU_PROFILE("void Thread::yield()", " ", TAU_USER);
  ThreadImpl::yield();
}

/**
 * Possibly suspends the current thread for the rest of the current epoch, if
 * it has run for enough this epoch.  This is especially important for the
 * simple thread implementation, which relies on cooperative yields like this.
 */
INLINE void Thread::
consider_yield() {
  TAU_PROFILE("void Thread::consider_yield()", " ", TAU_USER);
  ThreadImpl::consider_yield();
}

/**
 * Returns true if the thread has been started, false if it has not, or if
 * join() has already been called.
 */
INLINE bool Thread::
is_started() const {
  return _started;
}

/**
 * Returns the value of joinable that was passed to the start() call.
 */
INLINE bool Thread::
is_joinable() const {
  return _joinable;
}

/**
 * Blocks the calling process until the thread terminates.  If the thread has
 * already terminated, this returns immediately.
 */
INLINE void Thread::
join() {
  TAU_PROFILE("void Thread::join()", " ", TAU_USER);
  if (_started) {
    _impl.join();
    _started = false;
  }
}

/**
 * Indicates that this thread should run as soon as possible, preemptying any
 * other threads that may be scheduled to run.  This may not be implemented on
 * every platform.
 */
INLINE void Thread::
preempt() {
  if (_started) {
    _impl.preempt();
  }
}

/**
 * Returns the task currently executing on this thread (via the
 * AsyncTaskManager), if any, or NULL if the thread is not currently servicing
 * a task.
 */
INLINE TypedReferenceCount *Thread::
get_current_task() const {
  return (TypedReferenceCount *)_current_task;
}

/**
 * Stores a Python index to be associated with this thread.  This is used
 * internally by the thread module; you should not need to call this directly.
 */
INLINE void Thread::
set_python_index(int python_index) {
  _python_index = python_index;
}

/**
 * Should be called by the main thread just before exiting the program, this
 * blocks until any remaining thread cleanup has finished.
 */
INLINE void Thread::
prepare_for_exit() {
  ThreadImpl::prepare_for_exit();
}

#ifdef ANDROID
/**
 * Enables interaction with the Java VM on Android.  Returns null if the
 * thread is not attached to the Java VM (or bind_thread was not called).
 */
INLINE JNIEnv *Thread::
get_jni_env() const {
  return _impl.get_jni_env();
}
#endif

/**
 * Stores a PStats index to be associated with this thread.  This is used
 * internally by the PStatClient; you should not need to call this directly.
 */
INLINE void Thread::
set_pstats_index(int pstats_index) {
  _pstats_index = pstats_index;
}

/**
 * Stores a PStats callback to be associated with this thread.  This is used
 * internally by the PStatClient; you should not need to call this directly.
 */
INLINE void Thread::
set_pstats_callback(Thread::PStatsCallback *pstats_callback) {
  _pstats_callback = pstats_callback;
}

/**
 * Returns the PStats callback associated with this thread, or NULL if no
 * callback has yet been associated with this thread.  This is used internally
 * by the PStatClient; you should not need to call this directly.
 */
INLINE Thread::PStatsCallback *Thread::
get_pstats_callback() const {
  return _pstats_callback;
}

INLINE std::ostream &
operator << (std::ostream &out, const Thread &thread) {
  thread.output(out);
  return out;
}
