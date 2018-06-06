/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file pipelineCyclerDummyImpl.h
 * @author drose
 * @date 2006-01-31
 */

#ifndef PIPELINECYCLERDUMMYIMPL_H
#define PIPELINECYCLERDUMMYIMPL_H

#include "pandabase.h"

#if defined(DO_PIPELINING) && !defined(HAVE_THREADS)

#include "cycleData.h"
#include "pipeline.h"
#include "pointerTo.h"

/**
 * This is a simple, single-threaded-only implementation of
 * PipelineCyclerBase.  It is only compiled when DO_PIPELINING is defined, but
 * threading is not available, which is usually the case only in development
 * mode.
 *
 * This implmentation is similar in principle to PipelineCyclerTrivialImpl,
 * except it does basic sanity checking to ensure that you use the interface
 * in a reasonable way consistent with its design (e.g., read() is balanced
 * with release_read(), etc.).
 *
 * This is defined as a struct instead of a class, mainly to be consistent
 * with PipelineCyclerTrivialImpl.
 */
struct EXPCL_PANDA_PIPELINE PipelineCyclerDummyImpl {
public:
  INLINE PipelineCyclerDummyImpl(CycleData *initial_data, Pipeline *pipeline = nullptr);
  INLINE PipelineCyclerDummyImpl(const PipelineCyclerDummyImpl &copy);
  INLINE void operator = (const PipelineCyclerDummyImpl &copy);
  INLINE ~PipelineCyclerDummyImpl();

  INLINE void acquire(Thread *current_thread = nullptr);
  INLINE void release();

  INLINE const CycleData *read_unlocked(Thread *current_thread) const;
  INLINE const CycleData *read(Thread *current_thread) const;
  INLINE void increment_read(const CycleData *pointer) const;
  INLINE void release_read(const CycleData *pointer) const;

  INLINE CycleData *write(Thread *current_thread);
  INLINE CycleData *write_upstream(bool force_to_0, Thread *current_thread);
  INLINE CycleData *elevate_read(const CycleData *pointer, Thread *current_thread);
  INLINE CycleData *elevate_read_upstream(const CycleData *pointer, bool force_to_0, Thread *current_thread);
  INLINE void increment_write(CycleData *pointer) const;
  INLINE void release_write(CycleData *pointer);

  INLINE int get_num_stages();
  INLINE const CycleData *read_stage_unlocked(int pipeline_stage) const;
  INLINE const CycleData *read_stage(int pipeline_stage, Thread *current_thread) const;
  INLINE void release_read_stage(int pipeline_stage, const CycleData *pointer) const;
  INLINE CycleData *write_stage(int pipeline_stage, Thread *current_thread);
  INLINE CycleData *write_stage_upstream(int pipeline_stage, bool force_to_0, Thread *current_thread);
  INLINE CycleData *elevate_read_stage(int pipeline_stage, const CycleData *pointer, Thread *current_thread);
  INLINE CycleData *elevate_read_stage_upstream(int pipeline_stage, const CycleData *pointer,
                                                bool force_to_0, Thread *current_thread);
  INLINE void release_write_stage(int pipeline_stage, CycleData *pointer);

  INLINE TypeHandle get_parent_type() const;

  INLINE CycleData *cheat() const;
  INLINE int get_read_count() const;
  INLINE int get_write_count() const;

private:
  PT(CycleData) _data;
  Pipeline *_pipeline;
  short _read_count, _write_count;
  short _locked;
};

#include "pipelineCyclerDummyImpl.I"

#endif  // DO_PIPELINING && !HAVE_THREADS

#endif
