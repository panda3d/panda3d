// Filename: pipelineCycler.I
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


#ifdef DO_PIPELINING
// The following implementations are to support compiled-in pipeline
// sanity checks.

////////////////////////////////////////////////////////////////////
//     Function: PipelineCycler::Constructor (sanity-check)
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
//     Function: PipelineCycler::Copy Constructor (sanity-check)
//       Access: Public
//  Description: 
////////////////////////////////////////////////////////////////////
template<class CycleDataType>
INLINE PipelineCycler<CycleDataType>::
PipelineCycler(const PipelineCycler<CycleDataType> &copy) :
  // In the DO_PIPELINING copy constructor, the pointer value is ignored.
  PipelineCyclerBase(NULL, copy)
{
}

////////////////////////////////////////////////////////////////////
//     Function: PipelineCycler::Copy Assignment (sanity-check)
//       Access: Public
//  Description: 
////////////////////////////////////////////////////////////////////
template<class CycleDataType>
INLINE void PipelineCycler<CycleDataType>::
operator = (const PipelineCycler<CycleDataType> &copy) {
  PipelineCyclerBase::operator = (copy);
}

////////////////////////////////////////////////////////////////////
//     Function: PipelineCycler::read (sanity-check)
//       Access: Public
//  Description: See PipelineCyclerBase::read().
////////////////////////////////////////////////////////////////////
template<class CycleDataType>
INLINE const CycleDataType *PipelineCycler<CycleDataType>::
read() const {
  return (const CycleDataType *)PipelineCyclerBase::read();
}

////////////////////////////////////////////////////////////////////
//     Function: PipelineCycler::write (sanity-check)
//       Access: Public
//  Description: See PipelineCyclerBase::write().
////////////////////////////////////////////////////////////////////
template<class CycleDataType>
INLINE CycleDataType *PipelineCycler<CycleDataType>::
write() {
  return (CycleDataType *)PipelineCyclerBase::write();
}

////////////////////////////////////////////////////////////////////
//     Function: PipelineCycler::elevate_read (sanity-check)
//       Access: Public
//  Description: See PipelineCyclerBase::elevate_read().
////////////////////////////////////////////////////////////////////
template<class CycleDataType>
INLINE CycleDataType *PipelineCycler<CycleDataType>::
elevate_read(const CycleDataType *pointer) {
  return (CycleDataType *)PipelineCyclerBase::elevate_read(pointer);
}

////////////////////////////////////////////////////////////////////
//     Function: PipelineCycler::write_stage (sanity-check)
//       Access: Public
//  Description: See PipelineCyclerBase::write_stage().
////////////////////////////////////////////////////////////////////
template<class CycleDataType>
INLINE CycleDataType *PipelineCycler<CycleDataType>::
write_stage(int n) {
  return (CycleDataType *)PipelineCyclerBase::write_stage(n);
}

////////////////////////////////////////////////////////////////////
//     Function: PipelineCycler::cheat (sanity-check)
//       Access: Public
//  Description: Returns a pointer without counting it.  This is only
//               intended for use as the return value for certain
//               nassertr() functions, so the application can recover
//               after a failure to manage the read and write pointers
//               correctly.  You should never call this function
//               directly.
////////////////////////////////////////////////////////////////////
template<class CycleDataType>
INLINE CycleDataType *PipelineCycler<CycleDataType>::
cheat() const {
  return (CycleDataType *)PipelineCyclerBase::cheat();
}

#else  // !DO_PIPELINING
// The following implementations are provided for when pipelining is
// not compiled in.  They are trivial functions that do as little as
// possible.

////////////////////////////////////////////////////////////////////
//     Function: PipelineCycler::Constructor (trivial)
//       Access: Public
//  Description: 
////////////////////////////////////////////////////////////////////
template<class CycleDataType>
INLINE PipelineCycler<CycleDataType>::
PipelineCycler(Pipeline *pipeline) :
  PipelineCyclerBase(&_typed_data, pipeline)
{
}

////////////////////////////////////////////////////////////////////
//     Function: PipelineCycler::Copy Constructor (trivial)
//       Access: Public
//  Description: 
////////////////////////////////////////////////////////////////////
template<class CycleDataType>
INLINE PipelineCycler<CycleDataType>::
PipelineCycler(const PipelineCycler<CycleDataType> &copy) :
  PipelineCyclerBase(&_typed_data, copy),
  _typed_data(copy._typed_data)
{
}

////////////////////////////////////////////////////////////////////
//     Function: PipelineCycler::Copy Assignment (trivial)
//       Access: Public
//  Description: 
////////////////////////////////////////////////////////////////////
template<class CycleDataType>
INLINE void PipelineCycler<CycleDataType>::
operator = (const PipelineCycler<CycleDataType> &copy) {
  PipelineCyclerBase::operator = (copy);
  _typed_data = copy._typed_data;
}

////////////////////////////////////////////////////////////////////
//     Function: PipelineCycler::read (trivial)
//       Access: Public
//  Description: See PipelineCyclerBase::read().
////////////////////////////////////////////////////////////////////
template<class CycleDataType>
INLINE const CycleDataType *PipelineCycler<CycleDataType>::
read() const {
  return &_typed_data;
}

////////////////////////////////////////////////////////////////////
//     Function: PipelineCycler::write (trivial)
//       Access: Public
//  Description: See PipelineCyclerBase::write().
////////////////////////////////////////////////////////////////////
template<class CycleDataType>
INLINE CycleDataType *PipelineCycler<CycleDataType>::
write() {
  return &_typed_data;
}

////////////////////////////////////////////////////////////////////
//     Function: PipelineCycler::elevate_read (trivial)
//       Access: Public
//  Description: See PipelineCyclerBase::elevate_read().
////////////////////////////////////////////////////////////////////
template<class CycleDataType>
INLINE CycleDataType *PipelineCycler<CycleDataType>::
elevate_read(const CycleDataType *) {
  return &_typed_data;
}

////////////////////////////////////////////////////////////////////
//     Function: PipelineCycler::write_stage (trivial)
//       Access: Public
//  Description: See PipelineCyclerBase::write_stage().
////////////////////////////////////////////////////////////////////
template<class CycleDataType>
INLINE CycleDataType *PipelineCycler<CycleDataType>::
write_stage(int) {
  return &_typed_data;
}

////////////////////////////////////////////////////////////////////
//     Function: PipelineCycler::cheat (trivial)
//       Access: Public
//  Description: Returns a pointer without counting it.  This is only
//               intended for use as the return value for certain
//               nassertr() functions, so the application can recover
//               after a failure to manage the read and write pointers
//               correctly.  You should never call this function
//               directly.
////////////////////////////////////////////////////////////////////
template<class CycleDataType>
INLINE CycleDataType *PipelineCycler<CycleDataType>::
cheat() const {
  return (CycleDataType *)&_typed_data;
}


#endif   // DO_PIPELINING
