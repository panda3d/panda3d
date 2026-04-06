/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file pipelineCyclerTrivialImpl.h
 * @author drose
 * @date 2006-01-31
 */

#ifndef PIPELINECYCLERTRIVIALIMPL_H
#define PIPELINECYCLERTRIVIALIMPL_H

#include "pandabase.h"

#ifndef DO_PIPELINING

#include "thread.h"
#include "cycleData.h"

class Pipeline;

/**
 * This is the trivial, non-threaded implementation of PipelineCyclerBase.  It
 * is only compiled when DO_PIPELINING is not defined (which usually implies
 * that threading is not available).
 *
 * This implementation is designed to do as little as possible, and to compile
 * to nothing, or almost nothing.  It doesn't actually support pipelining in
 * any way.  It doesn't even perform any sanity checks to speak of.  It's
 * designed for a strictly single-threaded application, and its purpose is to
 * be as low-overhead as possible.
 *
 * We define this as a struct instead of a class to emphasize the importance
 * of byte placement within the object, so that the inherited struct's data is
 * likely to be placed by the compiler at the "this" pointer.
 */
struct EXPCL_PANDA_PIPELINE PipelineCyclerTrivialImpl {
public:
  INLINE PipelineCyclerTrivialImpl(CycleData *initial_data, Pipeline *pipeline = nullptr);
  PipelineCyclerTrivialImpl(const PipelineCyclerTrivialImpl &copy) = delete;
  ~PipelineCyclerTrivialImpl() = default;

  PipelineCyclerTrivialImpl &operator = (const PipelineCyclerTrivialImpl &copy) = delete;

  INLINE void acquire(Thread *current_thread = nullptr);
  INLINE void release();

  INLINE const CycleData *read_unlocked(Thread *current_thread) const;
  INLINE const CycleData *read(Thread *current_thread) const;
  INLINE void increment_read(const CycleData *pointer) const;
  INLINE void release_read(const CycleData *pointer) const;

  INLINE CycleData *write(Thread *current_thread);
  INLINE CycleData *write_upstream(bool force_to_0, Thread *current_thread);
  INLINE CycleData *elevate_read(const CycleData *pointer, Thread *current_thread);
  INLINE CycleData *elevate_read_upstream(const CycleData *pointer, bool force_to_0,
                                          Thread *current_thread);
  INLINE void increment_write(CycleData *pointer) const;
  INLINE void release_write(CycleData *pointer);

  INLINE int get_num_stages();
  INLINE const CycleData *read_stage_unlocked(int pipeline_stage) const;
  INLINE const CycleData *read_stage(int pipeline_stage, Thread *current_thread) const;
  INLINE void release_read_stage(int pipeline_stage, const CycleData *pointer) const;
  INLINE CycleData *write_stage(int pipeline_stage, Thread *current_thread);
  INLINE CycleData *write_stage_upstream(int pipeline_stage, bool force_to_0,
                                         Thread *current_thread);
  INLINE CycleData *elevate_read_stage(int pipeline_stage, const CycleData *pointer,
                                       Thread *current_thread);
  INLINE CycleData *elevate_read_stage_upstream(int pipeline_stage, const CycleData *pointer,
                                                bool force_to_0, Thread *current_thread);
  INLINE void release_write_stage(int pipeline_stage, CycleData *pointer);

  INLINE TypeHandle get_parent_type() const;

  INLINE CycleData *cheat() const;
  INLINE int get_read_count() const;
  INLINE int get_write_count() const;

  // In a trivial implementation, we only need to store the CycleData pointer.
  // Actually, we don't even need to do that, if we're lucky and the compiler
  // doesn't do anything funny with the struct layout.
  #ifndef SIMPLE_STRUCT_POINTERS
  CycleData *_data;
  #endif  // SIMPLE_STRUCT_POINTERS
};

#include "pipelineCyclerTrivialImpl.I"

#endif  // !DO_PIPELINING

#endif
