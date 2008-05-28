// Filename: cycleDataStageReader.h
// Created by:  drose (08Feb06)
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

#ifndef CYCLEDATASTAGEREADER_H
#define CYCLEDATASTAGEREADER_H

#include "pandabase.h"

#include "cycleData.h"
#include "pipelineCycler.h"

////////////////////////////////////////////////////////////////////
//       Class : CycleDataStageReader
// Description : This class is similar to CycleDataReader, except it
//               allows reading from a particular stage of the
//               pipeline.
////////////////////////////////////////////////////////////////////
template<class CycleDataType>
class CycleDataStageReader {
public:
  // By hiding this template from interrogate, we improve compile-time
  // speed and memory utilization.
#ifndef CPPPARSER
  INLINE CycleDataStageReader(const PipelineCycler<CycleDataType> &cycler, 
                              int stage, Thread *current_thread = Thread::get_current_thread());
  INLINE CycleDataStageReader(const CycleDataStageReader<CycleDataType> &copy);
  INLINE void operator = (const CycleDataStageReader<CycleDataType> &copy);

  INLINE ~CycleDataStageReader();

  INLINE const CycleDataType *operator -> () const;
  INLINE operator const CycleDataType * () const;

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

#include "cycleDataStageReader.I"

#endif
