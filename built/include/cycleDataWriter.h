/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file cycleDataWriter.h
 * @author drose
 * @date 2002-02-21
 */

#ifndef CYCLEDATAWRITER_H
#define CYCLEDATAWRITER_H

#include "pandabase.h"

#include "cycleData.h"
#include "pipelineCycler.h"
#include "cycleDataLockedReader.h"
#include "thread.h"

/**
 * This template class calls PipelineCycler::write() in the constructor and
 * PipelineCycler::release_write() in the destructor.  In the interim, it
 * provides a transparent read-write access to the CycleData.
 *
 * It exists as a syntactic convenience to access the data in the CycleData.
 * It also allows the whole system to compile down to nothing if DO_PIPELINING
 * is not defined.
 */
template<class CycleDataType>
class CycleDataWriter {
public:
  // By hiding this template from interrogate, we improve compile-time speed
  // and memory utilization.
#ifndef CPPPARSER
  INLINE CycleDataWriter(PipelineCycler<CycleDataType> &cycler,
                         Thread *current_thread = Thread::get_current_thread());
  INLINE CycleDataWriter(PipelineCycler<CycleDataType> &cycler, bool force_to_0,
                         Thread *current_thread = Thread::get_current_thread());
  INLINE CycleDataWriter(PipelineCycler<CycleDataType> &cycler,
                         CycleDataType *locked_cdata,
                         Thread *current_thread = Thread::get_current_thread());
  INLINE CycleDataWriter(const CycleDataWriter<CycleDataType> &copy);
  INLINE CycleDataWriter(CycleDataWriter<CycleDataType> &&from) noexcept;

  INLINE CycleDataWriter(PipelineCycler<CycleDataType> &cycler, CycleDataLockedReader<CycleDataType> &take_from);
  INLINE CycleDataWriter(PipelineCycler<CycleDataType> &cycler, CycleDataLockedReader<CycleDataType> &take_from, bool force_to_0);

  INLINE void operator = (CycleDataWriter<CycleDataType> &&from) noexcept;
  INLINE void operator = (const CycleDataWriter<CycleDataType> &copy);

  INLINE ~CycleDataWriter();

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
#else  // !DO_PIPELINING
  // This is all we need for the trivial, do-nothing implementation.
  CycleDataType *_pointer;
#endif  // DO_PIPELINING
#endif  // CPPPARSER
};

#include "cycleDataWriter.I"

#endif
