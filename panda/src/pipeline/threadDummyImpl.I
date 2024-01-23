/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file threadDummyImpl.I
 * @author drose
 * @date 2002-08-09
 */

/**
 *
 */
INLINE ThreadDummyImpl::
ThreadDummyImpl(Thread *) {
}

/**
 *
 */
INLINE ThreadDummyImpl::
~ThreadDummyImpl() {
}

/**
 * Called for the main thread only, which has been already started, to fill in
 * the values appropriate to that thread.
 */
void ThreadDummyImpl::
setup_main_thread() {
}

/**
 *
 */
INLINE bool ThreadDummyImpl::
start(ThreadPriority, bool) {
  return false;
}

/**
 *
 */
INLINE void ThreadDummyImpl::
join() {
}

/**
 *
 */
INLINE void ThreadDummyImpl::
preempt() {
}

/**
 *
 */
INLINE void ThreadDummyImpl::
prepare_for_exit() {
}

/**
 * Associates the indicated Thread object with the currently-executing thread,
 * unless a thread is already bound, in which case it is returned.
 * You should not call this directly; use Thread::bind_thread() instead.
 */
INLINE Thread *ThreadDummyImpl::
bind_thread(Thread *thread) {
  // This method shouldn't be called in the non-threaded case.
  nassertr(false, nullptr);
  return nullptr;
}

/**
 *
 */
INLINE bool ThreadDummyImpl::
is_threading_supported() {
  return false;
}

/**
 *
 */
INLINE bool ThreadDummyImpl::
is_true_threads() {
  return false;
}

/**
 *
 */
INLINE bool ThreadDummyImpl::
is_simple_threads() {
  return false;
}

/**
 *
 */
INLINE void ThreadDummyImpl::
sleep(double seconds) {
#ifdef _WIN32
  Sleep((int)(seconds * 1000));
#else
  struct timespec rqtp;
  rqtp.tv_sec = time_t(seconds);
  rqtp.tv_nsec = long((seconds - (double)rqtp.tv_sec) * 1000000000.0);
  nanosleep(&rqtp, nullptr);
#endif  // _WIN32
}

/**
 *
 */
INLINE void ThreadDummyImpl::
yield() {
}

/**
 *
 */
INLINE void ThreadDummyImpl::
consider_yield() {
}

/**
 *
 */
INLINE bool ThreadDummyImpl::
get_context_switches(size_t &, size_t &) {
  return false;
}

#ifdef ANDROID
/**
 * Returns the JNIEnv object for the current thread.
 */
INLINE JNIEnv *ThreadDummyImpl::
get_jni_env() const {
  return _jni_env;
}
#endif
