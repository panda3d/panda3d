// Filename: mutexImpl.h
// Created by:  drose (08Aug02)
//
////////////////////////////////////////////////////////////////////
//
// PANDA 3D SOFTWARE
// Copyright (c) Carnegie Mellon University.  All rights reserved.
//
// All use of this software is subject to the terms of the revised BSD
// license.  You should have received a copy of this license along
// with this source code in a file named "LICENSE."
//
////////////////////////////////////////////////////////////////////

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
typedef MutexWin32Impl ReMutexImpl;  // Win32 Mutexes are always reentrant.
#define HAVE_REMUTEXIMPL 1

#elif defined(THREAD_POSIX_IMPL)

#include "mutexPosixImpl.h"
typedef MutexPosixImpl MutexImpl;
typedef ReMutexPosixImpl ReMutexImpl;
#define HAVE_REMUTEXIMPL 1

#endif

#endif



