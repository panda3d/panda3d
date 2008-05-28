// Filename: threadImpl.h
// Created by:  drose (09Aug02)
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

#ifndef THREADIMPL_H
#define THREADIMPL_H

#include "pandabase.h"
#include "selectThreadImpl.h"

#if defined(THREAD_DUMMY_IMPL)

#include "threadDummyImpl.h"
typedef ThreadDummyImpl ThreadImpl;

#elif defined(THREAD_SIMPLE_IMPL)

#include "threadSimpleImpl.h"
typedef ThreadSimpleImpl ThreadImpl;

#elif defined(THREAD_WIN32_IMPL)

#include "threadWin32Impl.h"
typedef ThreadWin32Impl ThreadImpl;

#elif defined(THREAD_POSIX_IMPL)

#include "threadPosixImpl.h"
typedef ThreadPosixImpl ThreadImpl;

#endif

#endif
