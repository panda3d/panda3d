// Filename: pipelineCyclerBase.h
// Created by:  drose (21Feb02)
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

#ifndef PIPELINECYCLERBASE_H
#define PIPELINECYCLERBASE_H

#include "pandabase.h"
#include "selectThreadImpl.h"  // for THREADED_PIPELINE definition

#if defined(THREADED_PIPELINE)

// With DO_PIPELINING and threads available, we want the true cycler
// implementation.
#include "pipelineCyclerTrueImpl.h"
typedef PipelineCyclerTrueImpl PipelineCyclerBase;

#elif defined(DO_PIPELINING)

// With DO_PIPELINING but no threads available, we want the dummy,
// self-validating cycler implementation.
#include "pipelineCyclerDummyImpl.h"
typedef PipelineCyclerDummyImpl PipelineCyclerBase;

#else  // !DO_PIPELINING

// Without DO_PIPELINING, we only want the trivial, do-nothing
// implementation.
#include "pipelineCyclerTrivialImpl.h"
typedef PipelineCyclerTrivialImpl PipelineCyclerBase;

#endif  // DO_PIPELINING

#endif

