/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file mutexDummyImpl.I
 * @author drose
 * @date 2002-08-08
 */

/**
 *
 */
ALWAYS_INLINE void MutexDummyImpl::
lock() {
}

/**
 *
 */
ALWAYS_INLINE bool MutexDummyImpl::
try_lock() {
  return true;
}

/**
 *
 */
ALWAYS_INLINE void MutexDummyImpl::
unlock() {
}
