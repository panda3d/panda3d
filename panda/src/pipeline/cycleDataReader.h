/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file cycleDataReader.h
 * @author drose
 * @date 2002-02-21
 */

#ifndef CYCLEDATAREADER_H
#define CYCLEDATAREADER_H

#include "pandabase.h"

#include "cycleData.h"
#include "pipelineCycler.h"
#include "thread.h"

/**
 * This template class calls PipelineCycler::read_unlocked(), and then
 * provides a transparent read-only access to the CycleData.  It is used to
 * access the data quickly, without holding a lock, for a thread that does not
 * intend to modify the data and write it back out.  For cases where the data
 * might be subsequently modified, you should use CycleDataLockedReader.
 *
 * It exists as a syntactic convenience to access the data in the CycleData.
 * It also allows the whole system to compile down to nothing if DO_PIPELINING
 * is not defined.
 */
template<class CycleDataType>
class CycleDataReader {
public:
  // By hiding this template from interrogate, we improve compile-time speed
  // and memory utilization.
#ifndef CPPPARSER
  INLINE CycleDataReader(const PipelineCycler<CycleDataType> &cycler,
                         Thread *current_thread = Thread::get_current_thread());
  INLINE CycleDataReader(const CycleDataReader<CycleDataType> &copy);
  INLINE void operator = (const CycleDataReader<CycleDataType> &copy);

  INLINE ~CycleDataReader();

  INLINE const CycleDataType *operator -> () const;
  INLINE operator const CycleDataType * () const;
  INLINE const CycleDataType *p() const;

  INLINE Thread *get_current_thread() const;

private:
#ifdef DO_PIPELINING
  // This is the data stored for a real pipelining implementation.
  const PipelineCycler<CycleDataType> *_cycler;
  Thread *_current_thread;
  const CycleDataType *_pointer;
#else  // !DO_PIPELINING
  // This is all we need for the trivial, do-nothing implementation.
  const CycleDataType *_pointer;
#endif  // DO_PIPELINING
#endif  // CPPPARSER
};

#include "cycleDataReader.I"

#endif
