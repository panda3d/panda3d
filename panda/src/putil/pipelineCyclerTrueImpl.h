// Filename: pipelineCyclerTrueImpl.h
// Created by:  drose (31Jan06)
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

#ifndef PIPELINECYCLERTRUEIMPL_H
#define PIPELINECYCLERTRUEIMPL_H

#include "pandabase.h"

#if defined(DO_PIPELINING) && defined(HAVE_THREADS)

#include "cycleData.h"
#include "pointerTo.h"
#include "thread.h"
#include "reMutex.h"

class Pipeline;

////////////////////////////////////////////////////////////////////
//       Class : PipelineCyclerTrueImpl
// Description : This is the true, threaded implementation of
//               PipelineCyclerBase.  It is only compiled when
//               threading is available and DO_PIPELINING is defined.
//
//               This implementation is designed to do the actual work
//               of cycling the data through a pipeline, and returning
//               the actual CycleData appropriate to the current
//               thread's pipeline stage.
//
//               This is defined as a struct instead of a class,
//               mainly to be consistent with
//               PipelineCyclerTrivialImpl.
////////////////////////////////////////////////////////////////////
struct EXPCL_PANDA PipelineCyclerTrueImpl {
public:
  PipelineCyclerTrueImpl(CycleData *initial_data, Pipeline *pipeline = NULL);
  PipelineCyclerTrueImpl(const PipelineCyclerTrueImpl &copy);
  void operator = (const PipelineCyclerTrueImpl &copy);
  ~PipelineCyclerTrueImpl();

  INLINE const CycleData *read() const;
  INLINE void increment_read(const CycleData *pointer) const;
  INLINE void release_read(const CycleData *pointer) const;

  INLINE CycleData *write();
  INLINE CycleData *elevate_read(const CycleData *pointer);
  INLINE void release_write(CycleData *pointer);

  INLINE int get_num_stages();
  INLINE bool is_stage_unique(int n) const;
  CycleData *write_stage(int n);
  INLINE void release_write_stage(int n, CycleData *pointer);

  INLINE CycleData *cheat() const;
  INLINE int get_read_count() const;
  INLINE int get_write_count() const;

private:
  void cycle();
  void set_num_stages(int num_stages);

private:
  Pipeline *_pipeline;

  class StageData {
  public:
    PT(CycleData) _cycle_data;
  };

  StageData *_data;
  int _num_stages;

  ReMutex _lock;

  friend class Pipeline;
};

#include "pipelineCyclerTrueImpl.I"

#endif  // DO_PIPELINING && HAVE_THREADS

#endif

