// Filename: pipelineCycler.I
// Created by:  drose (21Feb02)
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


#ifdef DO_PIPELINING
// The following implementations are to support the
// PipelineCyclerDummyImpl or the PipelineCyclerTrueImpl.

////////////////////////////////////////////////////////////////////
//     Function: PipelineCycler::Constructor (dummy or true)
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
//     Function: PipelineCycler::Copy Constructor (dummy or true)
//       Access: Public
//  Description: 
////////////////////////////////////////////////////////////////////
template<class CycleDataType>
INLINE PipelineCycler<CycleDataType>::
PipelineCycler(const PipelineCycler<CycleDataType> &copy) :
  PipelineCyclerBase(copy)
{
}

////////////////////////////////////////////////////////////////////
//     Function: PipelineCycler::Copy Assignment (dummy or true)
//       Access: Public
//  Description: 
////////////////////////////////////////////////////////////////////
template<class CycleDataType>
INLINE void PipelineCycler<CycleDataType>::
operator = (const PipelineCycler<CycleDataType> &copy) {
  PipelineCyclerBase::operator = (copy);
}

////////////////////////////////////////////////////////////////////
//     Function: PipelineCycler::read_unlocked (dummy or true)
//       Access: Public
//  Description: See PipelineCyclerBase::read_unlocked().
////////////////////////////////////////////////////////////////////
template<class CycleDataType>
INLINE const CycleDataType *PipelineCycler<CycleDataType>::
read_unlocked(Thread *current_thread) const {
  return (const CycleDataType *)PipelineCyclerBase::read_unlocked(current_thread);
}

////////////////////////////////////////////////////////////////////
//     Function: PipelineCycler::read (dummy or true)
//       Access: Public
//  Description: See PipelineCyclerBase::read().
////////////////////////////////////////////////////////////////////
template<class CycleDataType>
INLINE const CycleDataType *PipelineCycler<CycleDataType>::
read(Thread *current_thread) const {
  return (const CycleDataType *)PipelineCyclerBase::read(current_thread);
}

////////////////////////////////////////////////////////////////////
//     Function: PipelineCycler::write (dummy or true)
//       Access: Public
//  Description: See PipelineCyclerBase::write().
////////////////////////////////////////////////////////////////////
template<class CycleDataType>
INLINE CycleDataType *PipelineCycler<CycleDataType>::
write(Thread *current_thread) {
  return (CycleDataType *)PipelineCyclerBase::write(current_thread);
}

////////////////////////////////////////////////////////////////////
//     Function: PipelineCycler::write_upstream (dummy or true)
//       Access: Public
//  Description: See PipelineCyclerBase::write_upstream().
////////////////////////////////////////////////////////////////////
template<class CycleDataType>
INLINE CycleDataType *PipelineCycler<CycleDataType>::
write_upstream(bool force_to_0, Thread *current_thread) {
  return (CycleDataType *)PipelineCyclerBase::write_upstream(force_to_0, current_thread);
}

////////////////////////////////////////////////////////////////////
//     Function: PipelineCycler::elevate_read (dummy or true)
//       Access: Public
//  Description: See PipelineCyclerBase::elevate_read().
////////////////////////////////////////////////////////////////////
template<class CycleDataType>
INLINE CycleDataType *PipelineCycler<CycleDataType>::
elevate_read(const CycleDataType *pointer, Thread *current_thread) {
  return (CycleDataType *)PipelineCyclerBase::elevate_read(pointer, current_thread);
}

////////////////////////////////////////////////////////////////////
//     Function: PipelineCycler::elevate_read_upstream (dummy or true)
//       Access: Public
//  Description: See PipelineCyclerBase::elevate_read_upstream().
////////////////////////////////////////////////////////////////////
template<class CycleDataType>
INLINE CycleDataType *PipelineCycler<CycleDataType>::
elevate_read_upstream(const CycleDataType *pointer, bool force_to_0,
                      Thread *current_thread) {
  return (CycleDataType *)PipelineCyclerBase::elevate_read_upstream(pointer, force_to_0, current_thread);
}

////////////////////////////////////////////////////////////////////
//     Function: PipelineCycler::read_stage_unlocked (dummy or true)
//       Access: Public
//  Description: See PipelineCyclerBase::read_stage_unlocked().
////////////////////////////////////////////////////////////////////
template<class CycleDataType>
INLINE const CycleDataType *PipelineCycler<CycleDataType>::
read_stage_unlocked(int pipeline_stage) const {
  return (const CycleDataType *)PipelineCyclerBase::read_stage_unlocked(pipeline_stage);
}

////////////////////////////////////////////////////////////////////
//     Function: PipelineCycler::read_stage (dummy or true)
//       Access: Public
//  Description: See PipelineCyclerBase::read_stage().
////////////////////////////////////////////////////////////////////
template<class CycleDataType>
INLINE const CycleDataType *PipelineCycler<CycleDataType>::
read_stage(int pipeline_stage, Thread *current_thread) const {
  return (const CycleDataType *)PipelineCyclerBase::read_stage(pipeline_stage, current_thread);
}

////////////////////////////////////////////////////////////////////
//     Function: PipelineCycler::elevate_read_stage (dummy or true)
//       Access: Public
//  Description: See PipelineCyclerBase::elevate_read_stage().
////////////////////////////////////////////////////////////////////
template<class CycleDataType>
INLINE CycleDataType *PipelineCycler<CycleDataType>::
elevate_read_stage(int pipeline_stage, const CycleDataType *pointer, 
                   Thread *current_thread) {
  return (CycleDataType *)PipelineCyclerBase::elevate_read_stage(pipeline_stage, pointer, current_thread);
}

////////////////////////////////////////////////////////////////////
//     Function: PipelineCycler::elevate_read_stage_upstream (dummy or true)
//       Access: Public
//  Description: See PipelineCyclerBase::elevate_read_stage_upstream().
////////////////////////////////////////////////////////////////////
template<class CycleDataType>
INLINE CycleDataType *PipelineCycler<CycleDataType>::
elevate_read_stage_upstream(int pipeline_stage, const CycleDataType *pointer, 
                            bool force_to_0, Thread *current_thread) {
  return (CycleDataType *)PipelineCyclerBase::elevate_read_stage_upstream(pipeline_stage, pointer, force_to_0, current_thread);
}

////////////////////////////////////////////////////////////////////
//     Function: PipelineCycler::write_stage (dummy or true)
//       Access: Public
//  Description: See PipelineCyclerBase::write_stage().
////////////////////////////////////////////////////////////////////
template<class CycleDataType>
INLINE CycleDataType *PipelineCycler<CycleDataType>::
write_stage(int pipeline_stage, Thread *current_thread) {
  return (CycleDataType *)PipelineCyclerBase::write_stage(pipeline_stage, current_thread);
}

////////////////////////////////////////////////////////////////////
//     Function: PipelineCycler::write_stage_upstream (dummy or true)
//       Access: Public
//  Description: See PipelineCyclerBase::write_stage_upstream().
////////////////////////////////////////////////////////////////////
template<class CycleDataType>
INLINE CycleDataType *PipelineCycler<CycleDataType>::
write_stage_upstream(int pipeline_stage, bool force_to_0, 
                     Thread *current_thread) {
  return (CycleDataType *)PipelineCyclerBase::write_stage_upstream(pipeline_stage, force_to_0, current_thread);
}

////////////////////////////////////////////////////////////////////
//     Function: PipelineCycler::cheat (dummy or true)
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
  PipelineCyclerBase(&_typed_data),
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
  _typed_data = copy._typed_data;
}

////////////////////////////////////////////////////////////////////
//     Function: PipelineCycler::read_unlocked (trivial)
//       Access: Public
//  Description: See PipelineCyclerBase::read_unlocked().
////////////////////////////////////////////////////////////////////
template<class CycleDataType>
INLINE const CycleDataType *PipelineCycler<CycleDataType>::
read_unlocked(Thread *) const {
  return &_typed_data;
}

////////////////////////////////////////////////////////////////////
//     Function: PipelineCycler::read (trivial)
//       Access: Public
//  Description: See PipelineCyclerBase::read().
////////////////////////////////////////////////////////////////////
template<class CycleDataType>
INLINE const CycleDataType *PipelineCycler<CycleDataType>::
read(Thread *) const {
  return &_typed_data;
}

////////////////////////////////////////////////////////////////////
//     Function: PipelineCycler::write (trivial)
//       Access: Public
//  Description: See PipelineCyclerBase::write().
////////////////////////////////////////////////////////////////////
template<class CycleDataType>
INLINE CycleDataType *PipelineCycler<CycleDataType>::
write(Thread *) {
  return &_typed_data;
}

////////////////////////////////////////////////////////////////////
//     Function: PipelineCycler::write_upstream (trivial)
//       Access: Public
//  Description: See PipelineCyclerBase::write_upstream().
////////////////////////////////////////////////////////////////////
template<class CycleDataType>
INLINE CycleDataType *PipelineCycler<CycleDataType>::
write_upstream(bool, Thread *) {
  return &_typed_data;
}

////////////////////////////////////////////////////////////////////
//     Function: PipelineCycler::elevate_read (trivial)
//       Access: Public
//  Description: See PipelineCyclerBase::elevate_read().
////////////////////////////////////////////////////////////////////
template<class CycleDataType>
INLINE CycleDataType *PipelineCycler<CycleDataType>::
elevate_read(const CycleDataType *, Thread *) {
  return &_typed_data;
}

////////////////////////////////////////////////////////////////////
//     Function: PipelineCycler::elevate_read_upstream (trivial)
//       Access: Public
//  Description: See PipelineCyclerBase::elevate_read_upstream().
////////////////////////////////////////////////////////////////////
template<class CycleDataType>
INLINE CycleDataType *PipelineCycler<CycleDataType>::
elevate_read_upstream(const CycleDataType *, bool, Thread *) {
  return &_typed_data;
}

////////////////////////////////////////////////////////////////////
//     Function: PipelineCycler::read_stage_unlocked (trivial)
//       Access: Public
//  Description: See PipelineCyclerBase::read_stage_unlocked().
////////////////////////////////////////////////////////////////////
template<class CycleDataType>
INLINE const CycleDataType *PipelineCycler<CycleDataType>::
read_stage_unlocked(int) const {
  return &_typed_data;
}

////////////////////////////////////////////////////////////////////
//     Function: PipelineCycler::read_stage (trivial)
//       Access: Public
//  Description: See PipelineCyclerBase::read_stage().
////////////////////////////////////////////////////////////////////
template<class CycleDataType>
INLINE const CycleDataType *PipelineCycler<CycleDataType>::
read_stage(int, Thread *) const {
  return &_typed_data;
}

////////////////////////////////////////////////////////////////////
//     Function: PipelineCycler::elevate_read_stage (trivial)
//       Access: Public
//  Description: See PipelineCyclerBase::elevate_read_stage().
////////////////////////////////////////////////////////////////////
template<class CycleDataType>
INLINE CycleDataType *PipelineCycler<CycleDataType>::
elevate_read_stage(int, const CycleDataType *, Thread *) {
  return &_typed_data;
}

////////////////////////////////////////////////////////////////////
//     Function: PipelineCycler::elevate_read_stage_upstream (trivial)
//       Access: Public
//  Description: See PipelineCyclerBase::elevate_read_stage_upstream().
////////////////////////////////////////////////////////////////////
template<class CycleDataType>
INLINE CycleDataType *PipelineCycler<CycleDataType>::
elevate_read_stage_upstream(int, const CycleDataType *, bool, Thread *) {
  return &_typed_data;
}

////////////////////////////////////////////////////////////////////
//     Function: PipelineCycler::write_stage (trivial)
//       Access: Public
//  Description: See PipelineCyclerBase::write_stage().
////////////////////////////////////////////////////////////////////
template<class CycleDataType>
INLINE CycleDataType *PipelineCycler<CycleDataType>::
write_stage(int, Thread *) {
  return &_typed_data;
}

////////////////////////////////////////////////////////////////////
//     Function: PipelineCycler::write_stage_upstream (trivial)
//       Access: Public
//  Description: See PipelineCyclerBase::write_stage_upstream().
////////////////////////////////////////////////////////////////////
template<class CycleDataType>
INLINE CycleDataType *PipelineCycler<CycleDataType>::
write_stage_upstream(int, bool, Thread *) {
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
