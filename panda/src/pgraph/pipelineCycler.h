// Filename: pipelineCycler.h
// Created by:  drose (21Feb02)
//
////////////////////////////////////////////////////////////////////
//
// PANDA 3D SOFTWARE
// Copyright (c) 2001, Disney Enterprises, Inc.  All rights reserved
//
// All use of this software is subject to the terms of the Panda 3d
// Software license.  You should have received a copy of this license
// along with this source code; you will also find a current copy of
// the license at http://www.panda3d.org/license.txt .
//
// To contact the maintainers of this program write to
// panda3d@yahoogroups.com .
//
////////////////////////////////////////////////////////////////////

#ifndef PIPELINECYCLER_H
#define PIPELINECYCLER_H

#include "pandabase.h"

#include "pipelineCyclerBase.h"

////////////////////////////////////////////////////////////////////
//       Class : PipelineCycler
// Description : This class maintains different copies of a page of
//               data between stages of the graphics pipeline (or any
//               other pipelining context).
//
//               The class object maintains up to n copies of a
//               CycleData structure, one for each stage of the
//               pipeline.  The head of the pipeline is responsible
//               for making changes to its copy, which are then cycled
//               through the pipeline at each frame.
//
//               To access the data, you must first ask for a readable
//               pointer.  In order to make changes to the data, you
//               must ask for a writable pointer.  Both kinds of
//               pointers should be released when you are done, as a
//               sanity check.  The CycleDataReader and
//               CycleDataWriter classes transparently handle this.
//
//               If pipelining support is not enabled at compile time
//               (that is, SUPPORT_PIPELINING is not defined), this
//               object compiles to a minimum object that presents the
//               same interface but with minimal runtime overhead.
//               (Actually, this isn't true yet, but it will be one
//               day.)
////////////////////////////////////////////////////////////////////
template<class CycleDataType>
class PipelineCycler : public PipelineCyclerBase {
public:
  INLINE PipelineCycler(Pipeline *pipeline = NULL);

  INLINE const CycleDataType *read() const;
  INLINE CycleDataType *write();
  INLINE CycleDataType *write_stage(int n);
};

#include "pipelineCycler.I"

#endif

