// Filename: cycleDataWriter.h
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

#ifndef CYCLEDATAWRITER_H
#define CYCLEDATAWRITER_H

#include "pandabase.h"

#include "cycleData.h"
#include "pipelineCycler.h"

////////////////////////////////////////////////////////////////////
//       Class : CycleDataWriter
// Description : This template class calls PipelineCycler::write() in
//               the constructor and PipelineCycler::release_write() in
//               the destructor.  In the interim, it provides a
//               transparent read-write access to the CycleData.
//
//               It exists as a syntactic convenience to access the
//               data in the CycleData.  It also allows the whole
//               system to compile down to nothing if
//               SUPPORT_PIPELINING is not defined.
////////////////////////////////////////////////////////////////////
template<class CycleDataType>
class CycleDataWriter {
public:
  INLINE CycleDataWriter(PipelineCycler<CycleDataType> &cycler);
  INLINE CycleDataWriter(const CycleDataWriter<CycleDataType> &copy);
  INLINE void operator = (const CycleDataWriter<CycleDataType> &copy);

  INLINE CycleDataWriter(PipelineCycler<CycleDataType> &cycler, CycleDataWriter<CycleDataType> &take_from);
  INLINE CycleDataWriter(PipelineCycler<CycleDataType> &cycler, CycleDataReader<CycleDataType> &take_from);

  INLINE ~CycleDataWriter();

  INLINE CycleDataType *operator -> ();
  INLINE const CycleDataType *operator -> () const;

  INLINE operator CycleDataType * ();

private:
#ifdef DO_PIPELINING
  // This is the data stored for a real pipelining implementation.
  PipelineCycler<CycleDataType> *_cycler;
  CycleDataType *_pointer;
#else  // !DO_PIPELINING
  // This is all we need for the trivial, do-nothing implementation.
  CycleDataType *_pointer;
#endif  // DO_PIPELINING
};

#include "cycleDataWriter.I"

#endif
