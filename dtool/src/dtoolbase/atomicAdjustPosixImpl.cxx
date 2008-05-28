// Filename: atomicAdjustPosixImpl.cxx
// Created by:  drose (10Feb06)
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

#include "selectThreadImpl.h"

#ifdef HAVE_POSIX_THREADS

#include "atomicAdjustPosixImpl.h"

pthread_mutex_t AtomicAdjustPosixImpl::_mutex = PTHREAD_MUTEX_INITIALIZER;

#endif  // HAVE_POSIX_THREADS
