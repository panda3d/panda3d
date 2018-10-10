/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file mutexTrueImpl.h
 * @author drose
 * @date 2007-06-19
 */

#ifndef MUTEXTRUEIMPL_H
#define MUTEXTRUEIMPL_H

#include "pandabase.h"
#include "mutexImpl.h"

// The MutexTrueImpl typedef is given here in the pipeline directory, and is
// used to implement Mutex and ReMutex (and, therefore, any downstream Mutex
// implementation).

// This is slightly different from the MutexImpl typedef, which is given up in
// dtoolbase, and is used standalone anywhere very low-level code needs to
// protect itself from mutual exclusion.

// The only difference between the two is in the case of THREAD_SIMPLE_IMPL.
// In this case, MutexImpl maps to MutexDummyImpl, while MutexTrueImpl maps to
// MutexSimpleImpl.  This distinction is necessary because we cannot define
// MutexSimpleImpl until we have defined the whole ThreadSimpleManager and
// related infrastructure.

#ifdef THREAD_SIMPLE_IMPL

#include "mutexSimpleImpl.h"
typedef MutexSimpleImpl MutexTrueImpl;
#undef HAVE_REMUTEXTRUEIMPL

#else  // THREAD_SIMPLE_IMPL

typedef MutexImpl MutexTrueImpl;
#if HAVE_REMUTEXIMPL
typedef ReMutexImpl ReMutexTrueImpl;
#define HAVE_REMUTEXTRUEIMPL 1

#elif MUTEX_SPINLOCK
// This is defined here because it needs code from pipeline.
#include "reMutexSpinlockImpl.h"
typedef ReMutexSpinlockImpl ReMutexTrueImpl;
#define HAVE_REMUTEXTRUEIMPL 1

#else
#undef HAVE_REMUTEXTRUEIMPL
#endif // HAVE_REMUTEXIMPL

#endif  // THREAD_SIMPLE_IMPL

#endif
