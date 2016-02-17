/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file mutexSimpleImpl.I
 * @author drose
 * @date 2007-06-19
 */

/**

 */
INLINE MutexSimpleImpl::
MutexSimpleImpl() {
}

/**

 */
INLINE MutexSimpleImpl::
~MutexSimpleImpl() {
}

/**

 */
INLINE void MutexSimpleImpl::
acquire() {
  if (!try_acquire()) {
    do_acquire();
  }
}

/**

 */
INLINE bool MutexSimpleImpl::
try_acquire() {
  if ((_flags & F_lock_count) != 0) {
    return false;
  }
  _flags |= F_lock_count;
  return true;
}

/**
 * Releases the mutex.  An immediate context switch might occur if there were
 * waiters on the mutex.
 */
INLINE void MutexSimpleImpl::
release() {
  nassertv((_flags & F_lock_count) != 0);
  _flags &= ~F_lock_count;

  if (_flags & F_has_waiters) {
    do_release();
  }
}

/**
 * Releases the mutex, without allowing a context switch to occur.
 */
INLINE void MutexSimpleImpl::
release_quietly() {
  nassertv((_flags & F_lock_count) != 0);
  _flags &= ~F_lock_count;

  if (_flags & F_has_waiters) {
    do_release_quietly();
  }
}
