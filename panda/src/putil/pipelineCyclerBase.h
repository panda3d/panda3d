// Filename: pipelineCyclerBase.h
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

#ifndef PIPELINECYCLERBASE_H
#define PIPELINECYCLERBASE_H

#include "pandabase.h"

#include "cycleData.h"
#include "pipeline.h"
#include "pointerTo.h"

////////////////////////////////////////////////////////////////////
//       Class : PipelineCyclerBase
// Description : This is the non-template part of the implementation
//               of PipelineCycler.  See PipelineCycler.
//
//               We define this as a struct instead of a class to
//               guarantee byte placement within the object, so that
//               (particularly for the trivial implementation) the
//               inherited struct's data is likely to be placed by the
//               compiler at the "this" pointer.
////////////////////////////////////////////////////////////////////
struct EXPCL_PANDA PipelineCyclerBase {
public:
  INLINE PipelineCyclerBase(CycleData *initial_data, Pipeline *pipeline = NULL);
  INLINE PipelineCyclerBase(CycleData *initial_data, const PipelineCyclerBase &copy);
  INLINE void operator = (const PipelineCyclerBase &copy);
  INLINE ~PipelineCyclerBase();

  INLINE const CycleData *read() const;
  INLINE void increment_read(const CycleData *pointer) const;
  INLINE void release_read(const CycleData *pointer) const;

  INLINE CycleData *write();
  INLINE CycleData *elevate_read(const CycleData *pointer);
  INLINE void release_write(CycleData *pointer);

  INLINE int get_num_stages();
  INLINE bool is_stage_unique(int n) const;
  INLINE CycleData *write_stage(int n);
  INLINE void release_write_stage(int n, CycleData *pointer);

  INLINE CycleData *cheat() const;
  INLINE int get_read_count() const;
  INLINE int get_write_count() const;

#ifdef DO_PIPELINING
  // This private data is only stored here if we have pipelining
  // compiled in.  Actually, this particular data is only used for
  // sanity checking the pipelining code; it doesn't do anything
  // useful.
private:
  PT(CycleData) _data;
  Pipeline *_pipeline;
  short _read_count, _write_count;

#else  // !DO_PIPELINING
  // In a trivial implementation, we only need to store the CycleData
  // pointer.  Actually, we don't even need to do that, if we're lucky
  // and the compiler doesn't do anything funny with the struct
  // layout.
  #ifndef SIMPLE_STRUCT_POINTERS
  CycleData *_data;
  #endif  // SIMPLE_STRUCT_POINTERS

#endif  // DO_PIPELINING
};

#include "pipelineCyclerBase.I"

#endif

