// Filename: atomicAdjust.h
// Created by:  drose (09Aug02)
//
////////////////////////////////////////////////////////////////////
//
// PANDA 3D SOFTWARE
// Copyright (c) 2001 - 2004, Disney Enterprises, Inc.  All rights reserved
//
// All use of this software is subject to the terms of the Panda 3d
// Software license.  You should have received a copy of this license
// along with this source code; you will also find a current copy of
// the license at http://etc.cmu.edu/panda3d/docs/license/ .
//
// To contact the maintainers of this program write to
// panda3d-general@lists.sourceforge.net .
//
////////////////////////////////////////////////////////////////////

#ifndef ATOMICADJUST_H
#define ATOMICADJUST_H

#include "dtoolbase.h"
#include "selectThreadImpl.h"

#if defined(THREAD_DUMMY_IMPL)

#include "atomicAdjustDummyImpl.h"
typedef AtomicAdjustDummyImpl AtomicAdjust;

#elif defined(__i386__) || defined(_M_IX86)
// For an i386 architecture, we'll always use the i386 implementation.
// It should be safe for any OS, and it might be a bit faster than
// any OS-provided calls.

#include "atomicAdjustI386Impl.h"
typedef AtomicAdjustI386Impl AtomicAdjust;

// These symbols are defined if the compare_and_exchange() methods are
// implemented natively, without recourse to external locks.  If these
// are not defined, users may elect to implement an operation with
// some other method than compare_and_exchange(), which might be
// faster.
#define HAVE_ATOMIC_COMPARE_AND_EXCHANGE 1
#define HAVE_ATOMIC_COMPARE_AND_EXCHANGE_PTR 1

#elif defined(THREAD_WIN32_IMPL)

#include "atomicAdjustWin32Impl.h"
typedef AtomicAdjustWin32Impl AtomicAdjust;

#define HAVE_ATOMIC_COMPARE_AND_EXCHANGE 1
#define HAVE_ATOMIC_COMPARE_AND_EXCHANGE_PTR 1

#elif defined(THREAD_LINUX_IMPL)

#error Linux native threads are currently implemented only for i386; use Posix threads instead.

#elif defined(THREAD_POSIX_IMPL)

#include "atomicAdjustPosixImpl.h"
typedef AtomicAdjustPosixImpl AtomicAdjust;

#elif defined(THREAD_NSPR_IMPL)

#include "atomicAdjustNsprImpl.h"
typedef AtomicAdjustNsprImpl AtomicAdjust;

#endif

#endif
