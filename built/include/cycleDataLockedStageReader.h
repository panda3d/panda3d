/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file cycleDataLockedStageReader.h
 * @author drose
 * @date 2006-04-30
 */

#ifndef CYCLEDATALOCKEDSTAGEREADER_H
#define CYCLEDATALOCKEDSTAGEREADER_H

#include "pandabase.h"

#include "cycleData.h"
#include "pipelineCycler.h"

/**
 * This class is similar to CycleDataLockedReader, except it allows reading
 * from a particular stage of the pipeline.
 */
template<class CycleDataType>
class CycleDataLockedStageReader {
public:
  // By hiding this template from interrogate, we improve compile-time speed
  // and memory utilization.
#ifndef CPPPARSER
  INLINE CycleDataLockedStageReader(const PipelineCycler<CycleDataType> &cycler,
                                    int stage, Thread *current_thread = Thread::get_current_thread());
  INLINE CycleDataLockedStageReader(const CycleDataLockedStageReader<CycleDataType> &copy);
  INLINE CycleDataLockedStageReader(CycleDataLockedStageReader<CycleDataType> &&from) noexcept;

  INLINE void operator = (const CycleDataLockedStageReader<CycleDataType> &copy);
  INLINE void operator = (CycleDataLockedStageReader<CycleDataType> &&from) noexcept;

  INLINE ~CycleDataLockedStageReader();

  INLINE const CycleDataType *operator -> () const;
  INLINE operator const CycleDataType * () const;

  INLINE const CycleDataType *take_pointer();
  INLINE Thread *get_current_thread() const;

private:
#ifdef DO_PIPELINING
  // This is the data stored for a real pipelining implementation.
  const PipelineCycler<CycleDataType> *_cycler;
  Thread *_current_thread;
  const CycleDataType *_pointer;
  int _stage;
#else  // !DO_PIPELINING
  // This is all we need for the trivial, do-nothing implementation.
  const CycleDataType *_pointer;
#endif  // DO_PIPELINING
#endif  // CPPPARSER
};

#include "cycleDataLockedStageReader.I"

#endif
