// Filename: cycleDataReader.h
// Created by:  drose (21Feb02)
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

#ifndef CYCLEDATAREADER_H
#define CYCLEDATAREADER_H

#include "pandabase.h"

#include "cycleData.h"
#include "pipelineCycler.h"

////////////////////////////////////////////////////////////////////
//       Class : CycleDataReader
// Description : This template class calls PipelineCycler::read() in
//               the constructor and PipelineCycler::release_read() in
//               the destructor.  In the interim, it provides a
//               transparent read-only access to the CycleData.
//
//               It exists as a syntactic convenience to access the
//               data in the CycleData.  It also allows the whole
//               system to compile down to nothing if
//               SUPPORT_PIPELINING is not defined.
////////////////////////////////////////////////////////////////////
template<class CycleDataType>
class CycleDataReader {
public:
  INLINE CycleDataReader(const PipelineCycler<CycleDataType> &cycler);
  INLINE CycleDataReader(const CycleDataReader<CycleDataType> &copy);
  INLINE void operator = (const CycleDataReader<CycleDataType> &copy);

  INLINE ~CycleDataReader();

  INLINE const CycleDataType *operator -> () const;
  INLINE operator const CycleDataType * () const;

  INLINE const CycleDataType *take_pointer();
  INLINE CycleDataType *elevate_to_write(PipelineCycler<CycleDataType> &cycler);

private:
#ifdef DO_PIPELINING
  // This is the data stored for a real pipelining implementation.
  const PipelineCycler<CycleDataType> *_cycler;
  const CycleDataType *_pointer;
  CycleDataType *_write_pointer;
#else  // !DO_PIPELINING
  // This is all we need for the trivial, do-nothing implementation.
  const CycleDataType *_pointer;
#endif  // DO_PIPELINING
};

#include "cycleDataReader.I"

#endif
