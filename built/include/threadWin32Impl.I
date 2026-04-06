/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file threadWin32Impl.I
 * @author drose
 * @date 2006-02-07
 */

/**
 *
 */
INLINE ThreadWin32Impl::
ThreadWin32Impl(Thread *parent_obj) :
  _cv(_mutex),
  _parent_obj(parent_obj),
  _profiling(0)
{
  _thread = 0;
  _joinable = false;
  _status = S_new;
}

/**
 *
 */
INLINE void ThreadWin32Impl::
preempt() {
}

/**
 *
 */
INLINE void ThreadWin32Impl::
prepare_for_exit() {
}

/**
 *
 */
INLINE bool ThreadWin32Impl::
is_threading_supported() {
  return true;
}

/**
 *
 */
INLINE bool ThreadWin32Impl::
is_true_threads() {
  return true;
}

/**
 *
 */
INLINE bool ThreadWin32Impl::
is_simple_threads() {
  return false;
}

/**
 *
 */
INLINE void ThreadWin32Impl::
yield() {
  Sleep(0);
}

/**
 *
 */
INLINE void ThreadWin32Impl::
consider_yield() {
}
