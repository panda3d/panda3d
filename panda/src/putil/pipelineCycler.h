// Filename: pipelineCycler.h
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
//
//               We define this as a struct instead of a class to
//               guarantee byte placement within the object, so that
//               (particularly for the trivial implementation) the
//               inherited struct's data is likely to be placed by the
//               compiler at the "this" pointer.
////////////////////////////////////////////////////////////////////
template<class CycleDataType>
struct PipelineCycler : public PipelineCyclerBase {
public:
  INLINE PipelineCycler(Pipeline *pipeline = NULL);
  INLINE PipelineCycler(const PipelineCycler<CycleDataType> &copy);
  INLINE void operator = (const PipelineCycler<CycleDataType> &copy);

  INLINE const CycleDataType *read() const;
  INLINE CycleDataType *write();
  INLINE CycleDataType *elevate_read(const CycleDataType *pointer);
  INLINE CycleDataType *write_stage(int n);

  INLINE CycleDataType *cheat() const;

#ifndef DO_PIPELINING
private:
  // If we are *not* compiling in support for pipelining, we just
  // store the CycleData object right here.  No pointers needed.
  CycleDataType _typed_data;
#endif  // !DO_PIPELINING
};

#include "pipelineCycler.I"

#endif

