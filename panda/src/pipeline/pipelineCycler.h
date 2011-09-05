// Filename: pipelineCycler.h
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

#ifndef PIPELINECYCLER_H
#define PIPELINECYCLER_H

#include "pandabase.h"
#include "pipelineCyclerBase.h"
#include "cyclerHolder.h"
#include "thread.h"

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
//               (that is, DO_PIPELINING is not defined), this object
//               compiles to a minimum object that presents the same
//               interface but with minimal runtime overhead.
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

  INLINE const CycleDataType *read_unlocked(Thread *current_thread) const;
  INLINE const CycleDataType *read(Thread *current_thread) const;
  INLINE CycleDataType *write(Thread *current_thread);
  INLINE CycleDataType *write_upstream(bool force_to_0, Thread *current_thread);
  INLINE CycleDataType *elevate_read(const CycleDataType *pointer, Thread *current_thread);
  INLINE CycleDataType *elevate_read_upstream(const CycleDataType *pointer, bool force_to_0, Thread *current_thread);

  INLINE const CycleDataType *read_stage_unlocked(int pipeline_stage) const;
  INLINE const CycleDataType *read_stage(int pipeline_stage, Thread *current_thread) const;
  INLINE CycleDataType *elevate_read_stage(int pipeline_stage, const CycleDataType *pointer, Thread *current_thread);
  INLINE CycleDataType *elevate_read_stage_upstream(int pipeline_stage, const CycleDataType *pointer, bool force_to_0, Thread *current_thread);
  INLINE CycleDataType *write_stage_upstream(int pipeline_stage, bool force_to_0, Thread *current_thread);
  INLINE CycleDataType *write_stage(int pipeline_stage, Thread *current_thread);

  INLINE CycleDataType *cheat() const;

#ifndef DO_PIPELINING
private:
  // If we are *not* compiling in support for pipelining, we just
  // store the CycleData object right here.  No pointers needed.
  CycleDataType _typed_data;
#endif  // !DO_PIPELINING
};

// These macros are handy for iterating through the set of pipeline
// stages.  They're particularly useful for updating cache values
// upstream of the current stage, or for removing bad pointers from
// all stages of the pipeline.  In each case, the variable
// pipeline_stage is defined within the loop to be the current stage
// of the pipeline traversed by the loop.
#ifdef DO_PIPELINING

// Iterates through all of the pipeline stages upstream of the current
// stage, but not including the current stage.
#define OPEN_ITERATE_UPSTREAM_ONLY(cycler, current_thread) {        \
    CyclerHolder cholder(cycler);                                   \
    int pipeline_stage;                                             \
    for (pipeline_stage = current_thread->get_pipeline_stage() - 1; \
         pipeline_stage >= 0;                                       \
         --pipeline_stage)

#define CLOSE_ITERATE_UPSTREAM_ONLY(cycler)     \
  }

// Iterates through all of the pipeline stages upstream of the current
// stage, and including the current stage.
#define OPEN_ITERATE_CURRENT_AND_UPSTREAM(cycler, current_thread) { \
    CyclerHolder cholder(cycler);                                   \
    int pipeline_stage;                                             \
    for (pipeline_stage = current_thread->get_pipeline_stage();     \
         pipeline_stage >= 0;                                       \
         --pipeline_stage)

#define CLOSE_ITERATE_CURRENT_AND_UPSTREAM(cycler)      \
  }

// As above, but without holding the cycler lock during the loop.
#define OPEN_ITERATE_CURRENT_AND_UPSTREAM_NOLOCK(cycler, current_thread) {  \
    int pipeline_stage;                                             \
    for (pipeline_stage = current_thread->get_pipeline_stage();    \
         pipeline_stage >= 0;                                       \
         --pipeline_stage)

#define CLOSE_ITERATE_CURRENT_AND_UPSTREAM_NOLOCK(cycler)   \
  }

// Iterates through all of the pipeline stages.
#define OPEN_ITERATE_ALL_STAGES(cycler) {                           \
    int pipeline_stage;                                             \
    for (pipeline_stage = (cycler).get_num_stages() - 1;            \
         pipeline_stage >= 0;                                       \
         --pipeline_stage)

#define CLOSE_ITERATE_ALL_STAGES(cycler)        \
  }

#else  // DO_PIPELINING

// These are trivial implementations of the above macros, defined when
// pipelining is not enabled, that simply operate on stage 0 without
// bothering to create a for loop.
#define OPEN_ITERATE_UPSTREAM_ONLY(cycler, current_thread)      \
  if (false) {                                  \
    const int pipeline_stage = -1;                   

#define CLOSE_ITERATE_UPSTREAM_ONLY(cycler)     \
  }

#define OPEN_ITERATE_CURRENT_AND_UPSTREAM(cycler, current_thread) {                     \
    const int pipeline_stage = 0;                                       \
    
#define CLOSE_ITERATE_CURRENT_AND_UPSTREAM(cycler)      \
  }

#define OPEN_ITERATE_CURRENT_AND_UPSTREAM_NOLOCK(cycler, current_thread) {  \
    const int pipeline_stage = 0;                                       \
    
#define CLOSE_ITERATE_CURRENT_AND_UPSTREAM_NOLOCK(cycler)   \
  }

#define OPEN_ITERATE_ALL_STAGES(cycler) {                               \
    const int pipeline_stage = 0;                                       \
    
#define CLOSE_ITERATE_ALL_STAGES(cycler)        \
  }

#endif  // DO_PIPELINING

#include "pipelineCycler.I"

#endif

