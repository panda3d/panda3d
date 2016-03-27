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

#ifdef WIN32_VC

#include "mutexWin32Impl.h"

/**
 *
 */
MutexWin32Impl::
MutexWin32Impl() {
  InitializeCriticalSectionAndSpinCount(&_lock, 4000);
}

#endif  // WIN32_VC
