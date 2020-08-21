/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file pipelineCyclerTrueImpl.h
 * @author drose
 * @date 2006-01-31
 */

#ifndef PIPELINECYCLERTRUEIMPL_H
#define PIPELINECYCLERTRUEIMPL_H

#include "pandabase.h"
#include "selectThreadImpl.h"  // for THREADED_PIPELINE definition

#ifdef THREADED_PIPELINE

#include "pipelineCyclerLinks.h"
#include "cycleData.h"
#include "pointerTo.h"
#include "nodePointerTo.h"
#include "thread.h"
#include "reMutex.h"
#include "reMutexHolder.h"

class Pipeline;

/**
 * This is the true, threaded implementation of PipelineCyclerBase.  It is
 * only compiled when threading is available and DO_PIPELINING is defined.
 *
 * This implementation is designed to do the actual work of cycling the data
 * through a pipeline, and returning the actual CycleData appropriate to the
 * current thread's pipeline stage.
 *
 * This is defined as a struct instead of a class, mainly to be consistent
 * with PipelineCyclerTrivialImpl.
 */
struct EXPCL_PANDA_PIPELINE PipelineCyclerTrueImpl : public PipelineCyclerLinks {
private:
  PipelineCyclerTrueImpl();
public:
  PipelineCyclerTrueImpl(CycleData *initial_data, Pipeline *pipeline = nullptr);
  PipelineCyclerTrueImpl(const PipelineCyclerTrueImpl &copy);
  void operator = (const PipelineCyclerTrueImpl &copy);
  ~PipelineCyclerTrueImpl();

  INLINE void acquire();
  INLINE void acquire(Thread *current_thread);
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
  CycleData *write_stage(int pipeline_stage, Thread *current_thread);
  CycleData *write_stage_upstream(int pipeline_stage, bool force_to_0,
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

public:
  // We redefine the ReMutex class, solely so we can define the output()
  // operator.  This is only useful for debugging, but does no harm in the
  // production case.
  class CyclerMutex : public ReMutex {
  public:
    INLINE CyclerMutex(PipelineCyclerTrueImpl *cycler);

#ifdef DEBUG_THREADS
    virtual void output(std::ostream &out) const;
    PipelineCyclerTrueImpl *_cycler;
#endif  // DEBUG_THREADS
  };

private:
  PT(CycleData) cycle();
  INLINE PT(CycleData) cycle_2();
  INLINE PT(CycleData) cycle_3();
  void set_num_stages(int num_stages);

private:
  Pipeline *_pipeline;

  // An array of PT(CycleData) objects representing the different copies of
  // the cycled data, one for each stage.
  class CycleDataNode : public MemoryBase {
  public:
    INLINE CycleDataNode();
    INLINE CycleDataNode(const CycleDataNode &copy);
    INLINE ~CycleDataNode();
    INLINE void operator = (const CycleDataNode &copy);

    NPT(CycleData) _cdata;
    int _writes_outstanding;
  };
  CycleDataNode *_data;
  int _num_stages;

  // This is 0 if it's clean, or set to Pipeline::_next_cycle_seq if it's
  // scheduled to be cycled during the next cycle() call.
  unsigned int _dirty;

  CyclerMutex _lock;

  friend class Pipeline;
};

#include "pipelineCyclerTrueImpl.I"

#endif  // THREADED_PIPELINE

#endif
