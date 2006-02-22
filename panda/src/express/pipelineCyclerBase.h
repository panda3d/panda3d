// Filename: pipelineCyclerBase.h
// Created by:  drose (21Feb02)
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

#ifndef PIPELINECYCLERBASE_H
#define PIPELINECYCLERBASE_H

#include "pandabase.h"

#ifdef DO_PIPELINING

#ifdef HAVE_THREADS

// With DO_PIPELINING and threads available, we want the true cycler
// implementation.
#include "pipelineCyclerTrueImpl.h"
typedef PipelineCyclerTrueImpl PipelineCyclerBase;

#else  // HAVE_THREADS

// With DO_PIPELINING but no threads available, we want the dummy,
// self-validating cycler implementation.
#include "pipelineCyclerDummyImpl.h"
typedef PipelineCyclerDummyImpl PipelineCyclerBase;

#endif // HAVE_THREADS

#else  // DO_PIPELINING

// Without DO_PIPELINING, we only want the trivial, do-nothing
// implementation.
#include "pipelineCyclerTrivialImpl.h"
typedef PipelineCyclerTrivialImpl PipelineCyclerBase;

#endif  // DO_PIPELINING

#endif

