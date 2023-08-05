/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file mutexWin32Impl.cxx
 * @author drose
 * @date 2006-02-07
 */

#include "selectThreadImpl.h"

#ifdef _WIN32

#include "mutexWin32Impl.h"

// The number of spins to do before suspending the thread.
static const unsigned int spin_count = 4000;

/**
 *
 */
ReMutexWin32Impl::
ReMutexWin32Impl() {
  InitializeCriticalSectionAndSpinCount(&_lock, spin_count);
}

#endif  // _WIN32
