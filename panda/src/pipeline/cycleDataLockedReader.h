/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file cycleDataLockedReader.h
 * @author drose
 * @date 2006-04-30
 */

#ifndef CYCLEDATALOCKEDREADER_H
#define CYCLEDATALOCKEDREADER_H

#include "pandabase.h"

#include "cycleData.h"
#include "pipelineCycler.h"
#include "thread.h"

/**
 * This template class calls PipelineCycler::read() in the constructor and
 * PipelineCycler::release_read() in the destructor.  In the interim, it
 * provides a transparent read-only access to the CycleData.
 *
 * Since a lock is held on the data while the instance of this class exists,
 * no other thread may modify any stage of the pipeline during that time.
 * Thus, this class is appropriate to use for cases in which you might want to
 * read and then modify the data.  It is possible to pass an instance of
 * CycleDataLockedReader to the CycleDataWriter constructor, which
 * automatically elevates the read lock into a write lock.
 *
 * It exists as a syntactic convenience to access the data in the CycleData.
 * It also allows the whole system to compile down to nothing if DO_PIPELINING
 * is not defined.
 */
template<class CycleDataType>
class CycleDataLockedReader {
public:
  // By hiding this template from interrogate, we improve compile-time speed
  // and memory utilization.
#ifndef CPPPARSER
  INLINE CycleDataLockedReader(const PipelineCycler<CycleDataType> &cycler,
                               Thread *current_thread = Thread::get_current_thread());
  INLINE CycleDataLockedReader(const CycleDataLockedReader<CycleDataType> &copy);
  INLINE CycleDataLockedReader(CycleDataLockedReader<CycleDataType> &&from) noexcept;

  INLINE void operator = (const CycleDataLockedReader<CycleDataType> &copy);
  INLINE void operator = (CycleDataLockedReader<CycleDataType> &&from) noexcept;

  INLINE ~CycleDataLockedReader();

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
  CycleDataType *_write_pointer;
#else  // !DO_PIPELINING
  // This is all we need for the trivial, do-nothing implementation.
  const CycleDataType *_pointer;
#endif  // DO_PIPELINING
#endif  // CPPPARSER
};

#include "cycleDataLockedReader.I"

#endif
