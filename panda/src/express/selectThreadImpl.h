// Filename: selectThreadImpl.h
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

#ifndef SELECTTHREADIMPL_H
#define SELECTTHREADIMPL_H

#include "pandabase.h"

////////////////////////////////////////////////////////////////////
// This file decides which of the core implementations of the various
// threading and locking implementations we should use, based on
// platform and/or available libraries.
////////////////////////////////////////////////////////////////////

#if !defined(HAVE_THREADS)

// With threading disabled, use the do-nothing implementation.
#define THREAD_DUMMY_IMPL 1

#elif defined(HAVE_NSPR)

// If NSPR is available, use that.
#define THREAD_NSPR_IMPL 1

#else

// This is a configuration error.  For some reason, HAVE_THREADS is
// defined but we don't have any way to implement it.
#error No thread implementation defined for platform.

#endif


#endif
