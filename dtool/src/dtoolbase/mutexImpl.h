/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file mutexImpl.h
 * @author drose
 * @date 2002-08-08
 */

#ifndef MUTEXIMPL_H
#define MUTEXIMPL_H

#include "dtoolbase.h"
#include "selectThreadImpl.h"

#if defined(THREAD_DUMMY_IMPL)||defined(THREAD_SIMPLE_IMPL)

#include "mutexDummyImpl.h"
typedef MutexDummyImpl MutexImpl;
typedef MutexDummyImpl ReMutexImpl;
#define HAVE_REMUTEXIMPL 1

#elif defined(MUTEX_SPINLOCK)

#include "mutexSpinlockImpl.h"
typedef MutexSpinlockImpl MutexImpl;
#undef HAVE_REMUTEXIMPL

#elif defined(THREAD_WIN32_IMPL)

#include "mutexWin32Impl.h"
typedef MutexWin32Impl MutexImpl;
typedef ReMutexWin32Impl ReMutexImpl;
#define HAVE_REMUTEXIMPL 1

#elif defined(THREAD_POSIX_IMPL)

#include "mutexPosixImpl.h"
typedef MutexPosixImpl MutexImpl;
typedef ReMutexPosixImpl ReMutexImpl;
#define HAVE_REMUTEXIMPL 1

#endif

// Also define what a true OS-provided lock will be, even if we don't have
// threading enabled in the build.  Sometimes we need to interface with an
// external program or something that wants real locks.
#if defined(WIN32_VC)
#include "mutexWin32Impl.h"
typedef MutexWin32Impl TrueMutexImpl;

#elif defined(HAVE_POSIX_THREADS)
#include "mutexPosixImpl.h"
typedef MutexPosixImpl TrueMutexImpl;

#else
// No true threads, sorry.  Better not try to use 'em.

#endif

#endif
