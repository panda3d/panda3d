// Filename: cycleDataStageReader.h
// Created by:  drose (08Feb06)
//
////////////////////////////////////////////////////////////////////
//
// PANDA 3D SOFTWARE
// Copyright (c) 2001 - 2004, Disney Enterprises, Inc.  All rights reserved
//
// All use of this software is subject to the terms of the Panda 3d
// Software license.  You should have received a copy of this license
// along with this source code; you will also find a current copy of
// the license at http://etc.cmu.edu/panda3d/docs/license/ .
//
// To contact the maintainers of this program write to
// panda3d-general@lists.sourceforge.net .
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
  INLINE CycleDataStageReader(const PipelineCycler<CycleDataType> &cycler, int stage);
  INLINE CycleDataStageReader(const CycleDataStageReader<CycleDataType> &copy);
  INLINE void operator = (const CycleDataStageReader<CycleDataType> &copy);

  INLINE ~CycleDataStageReader();

  INLINE const CycleDataType *operator -> () const;
  INLINE operator const CycleDataType * () const;

  INLINE const CycleDataType *take_pointer();

private:
#ifdef DO_PIPELINING
  // This is the data stored for a real pipelining implementation.
  const PipelineCycler<CycleDataType> *_cycler;
  const CycleDataType *_pointer;
  int _stage;
#else  // !DO_PIPELINING
  // This is all we need for the trivial, do-nothing implementation.
  const CycleDataType *_pointer;
#endif  // DO_PIPELINING
};

#include "cycleDataStageReader.I"

#endif
