// Filename: pipelineCycler.I
// Created by:  drose (21Feb02)
//
////////////////////////////////////////////////////////////////////
//
// PANDA 3D SOFTWARE
// Copyright (c) 2001, Disney Enterprises, Inc.  All rights reserved
//
// All use of this software is subject to the terms of the Panda 3d
// Software license.  You should have received a copy of this license
// along with this source code; you will also find a current copy of
// the license at http://www.panda3d.org/license.txt .
//
// To contact the maintainers of this program write to
// panda3d@yahoogroups.com .
//
////////////////////////////////////////////////////////////////////


////////////////////////////////////////////////////////////////////
//     Function: PipelineCycler::Constructor
//       Access: Public
//  Description: 
////////////////////////////////////////////////////////////////////
template<class CycleDataType>
INLINE PipelineCycler<CycleDataType>::
PipelineCycler(Pipeline *pipeline) :
  PipelineCyclerBase(new CycleDataType, pipeline)
{
}

////////////////////////////////////////////////////////////////////
//     Function: PipelineCycler::read
//       Access: Public
//  Description: See PipelineCyclerBase::read().
////////////////////////////////////////////////////////////////////
template<class CycleDataType>
INLINE const CycleDataType *PipelineCycler<CycleDataType>::
read() const {
  return (const CycleDataType *)PipelineCyclerBase::read();
}

////////////////////////////////////////////////////////////////////
//     Function: PipelineCycler::write
//       Access: Public
//  Description: See PipelineCyclerBase::write().
////////////////////////////////////////////////////////////////////
template<class CycleDataType>
INLINE CycleDataType *PipelineCycler<CycleDataType>::
write() {
  return (CycleDataType *)PipelineCyclerBase::write();
}

////////////////////////////////////////////////////////////////////
//     Function: PipelineCycler::write_stage
//       Access: Public
//  Description: See PipelineCyclerBase::write_stage().
////////////////////////////////////////////////////////////////////
template<class CycleDataType>
INLINE CycleDataType *PipelineCycler<CycleDataType>::
write_stage(int n) {
  return (CycleDataType *)PipelineCyclerBase::write_stage(n);
}
