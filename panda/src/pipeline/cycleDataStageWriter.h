/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file cycleDataStageWriter.h
 * @author drose
 * @date 2006-02-06
 */

#ifndef CYCLEDATASTAGEWRITER_H
#define CYCLEDATASTAGEWRITER_H

#include "pandabase.h"

#include "cycleData.h"
#include "pipelineCycler.h"
#include "cycleDataLockedStageReader.h"

/**
 * This class is similar to CycleDataWriter, except it allows writing to a
 * particular stage of the pipeline.  Usually this is used to implement
 * writing directly to an upstream pipeline value, to recompute a cached value
 * there (otherwise, the cached value would go away with the next pipeline
 * cycle).
 */
template<class CycleDataType>
class CycleDataStageWriter {
public:
  // By hiding this template from interrogate, we improve compile-time speed
  // and memory utilization.
#ifndef CPPPARSER
  INLINE CycleDataStageWriter(PipelineCycler<CycleDataType> &cycler, int stage,
                              Thread *current_thread = Thread::get_current_thread());
  INLINE CycleDataStageWriter(PipelineCycler<CycleDataType> &cycler, int stage,
                              bool force_to_0, Thread *current_thread = Thread::get_current_thread());

  INLINE CycleDataStageWriter(const CycleDataStageWriter<CycleDataType> &copy);
  INLINE CycleDataStageWriter(CycleDataStageWriter<CycleDataType> &&from) noexcept;

  INLINE CycleDataStageWriter(PipelineCycler<CycleDataType> &cycler, int stage,
                              CycleDataLockedStageReader<CycleDataType> &take_from);
  INLINE CycleDataStageWriter(PipelineCycler<CycleDataType> &cycler, int stage,
                              CycleDataLockedStageReader<CycleDataType> &take_from,
                              bool force_to_0);

  INLINE ~CycleDataStageWriter();

  INLINE void operator = (const CycleDataStageWriter<CycleDataType> &copy);
  INLINE void operator = (CycleDataStageWriter<CycleDataType> &&from) noexcept;

  INLINE CycleDataType *operator -> ();
  INLINE const CycleDataType *operator -> () const;

  INLINE operator CycleDataType * ();

  INLINE Thread *get_current_thread() const;

private:
#ifdef DO_PIPELINING
  // This is the data stored for a real pipelining implementation.
  PipelineCycler<CycleDataType> *_cycler;
  Thread *_current_thread;
  CycleDataType *_pointer;
  int _stage;
#else  // !DO_PIPELINING
  // This is all we need for the trivial, do-nothing implementation.
  CycleDataType *_pointer;
#endif  // DO_PIPELINING
#endif  // CPPPARSER
};

#include "cycleDataStageWriter.I"

#endif
