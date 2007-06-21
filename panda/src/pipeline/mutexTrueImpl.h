// Filename: mutexTrueImpl.h
// Created by:  drose (19Jun07)
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

#ifndef MUTEXTRUEIMPL_H
#define MUTEXTRUEIMPL_H

#include "pandabase.h"
#include "mutexImpl.h"

// The MutexTrueImpl typedef is given here in the pipeline directory,
// and is used to implement Mutex and ReMutex (and, therefore, any
// downstream Mutex implementation).

// This is slightly different from the MutexImpl typedef, which is
// given up in dtoolbase, and is used standalone anywhere very
// low-level code needs to protect itself from mutual exclusion.

// The only difference between the two is in the case of
// THREAD_SIMPLE_IMPL.  In this case, MutexImpl maps to
// MutexDummyImpl, while MutexTrueImpl maps to MutexSimpleImpl.  This
// distinction is necessary because we cannot define MutexSimpleImpl
// until we have defined the whole ThreadSimpleManager and related
// infrastructure.

#if defined(THREAD_SIMPLE_IMPL) && !defined(SIMPLE_THREADS_NO_IMPLICIT_YIELD)

#include "mutexSimpleImpl.h"
#include "reMutexSimpleImpl.h"
typedef MutexSimpleImpl MutexTrueImpl;
typedef ReMutexSimpleImpl ReMutexTrueImpl;

#else

typedef MutexImpl MutexTrueImpl;
typedef ReMutexImpl ReMutexTrueImpl;

#endif

#endif



