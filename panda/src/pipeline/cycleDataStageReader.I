// Filename: cycleDataStageReader.I
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

#ifndef CPPPARSER

#ifdef DO_PIPELINING
// This is the implementation for full support of pipelining (as well
// as the sanity-check only implementation).

////////////////////////////////////////////////////////////////////
//     Function: CycleDataStageReader::Constructor (full)
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
template<class CycleDataType>
INLINE CycleDataStageReader<CycleDataType>::
CycleDataStageReader(const PipelineCycler<CycleDataType> &cycler,
                     int stage, Thread *current_thread) :
  _cycler(&cycler),
  _current_thread(current_thread),
  _stage(stage)
{
  _pointer = _cycler->read_stage_unlocked(_stage);
}

////////////////////////////////////////////////////////////////////
//     Function: CycleDataStageReader::Copy Constructor (full)
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
template<class CycleDataType>
INLINE CycleDataStageReader<CycleDataType>::
CycleDataStageReader(const CycleDataStageReader<CycleDataType> &copy) :
  _cycler(copy._cycler),
  _current_thread(copy._current_thread),
  _pointer(copy._pointer),
  _stage(copy._stage)
{
}

////////////////////////////////////////////////////////////////////
//     Function: CycleDataStageReader::Copy Assignment (full)
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
template<class CycleDataType>
INLINE void CycleDataStageReader<CycleDataType>::
operator = (const CycleDataStageReader<CycleDataType> &copy) {
  nassertv(_current_thread == copy._current_thread);

  _cycler = copy._cycler;
  _pointer = copy._pointer;
  _stage = copy._stage;
}

////////////////////////////////////////////////////////////////////
//     Function: CycleDataStageReader::Destructor (full)
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
template<class CycleDataType>
INLINE CycleDataStageReader<CycleDataType>::
~CycleDataStageReader() {
}

////////////////////////////////////////////////////////////////////
//     Function: CycleDataStageReader::operator -> (full)
//       Access: Public
//  Description: This provides an indirect member access to the actual
//               CycleData data.
////////////////////////////////////////////////////////////////////
template<class CycleDataType>
INLINE const CycleDataType *CycleDataStageReader<CycleDataType>::
operator -> () const {
  return _pointer;
}

////////////////////////////////////////////////////////////////////
//     Function: CycleDataStageReader::Typecast pointer (full)
//       Access: Public
//  Description: This allows the CycleDataStageReader to be passed to any
//               function that expects a const CycleDataType pointer.
////////////////////////////////////////////////////////////////////
template<class CycleDataType>
INLINE CycleDataStageReader<CycleDataType>::
operator const CycleDataType * () const {
  return _pointer;
}

////////////////////////////////////////////////////////////////////
//     Function: CycleDataStageReader::get_current_thread (full)
//       Access: Public
//  Description: Returns the Thread pointer of the currently-executing
//               thread, as passed to the constructor of this object.
////////////////////////////////////////////////////////////////////
template<class CycleDataType>
INLINE Thread *CycleDataStageReader<CycleDataType>::
get_current_thread() const {
  return _current_thread;
}

#else  // !DO_PIPELINING
// This is the trivial, do-nothing implementation.

////////////////////////////////////////////////////////////////////
//     Function: CycleDataStageReader::Constructor (trivial)
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
template<class CycleDataType>
INLINE CycleDataStageReader<CycleDataType>::
CycleDataStageReader(const PipelineCycler<CycleDataType> &cycler, int,
                     Thread *) {
  _pointer = cycler.cheat();
}

////////////////////////////////////////////////////////////////////
//     Function: CycleDataStageReader::Copy Constructor (trivial)
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
template<class CycleDataType>
INLINE CycleDataStageReader<CycleDataType>::
CycleDataStageReader(const CycleDataStageReader<CycleDataType> &copy) :
  _pointer(copy._pointer)
{
}

////////////////////////////////////////////////////////////////////
//     Function: CycleDataStageReader::Copy Assignment (trivial)
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
template<class CycleDataType>
INLINE void CycleDataStageReader<CycleDataType>::
operator = (const CycleDataStageReader<CycleDataType> &copy) {
  _pointer = copy._pointer;
}

////////////////////////////////////////////////////////////////////
//     Function: CycleDataStageReader::Destructor (trivial)
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
template<class CycleDataType>
INLINE CycleDataStageReader<CycleDataType>::
~CycleDataStageReader() {
}

////////////////////////////////////////////////////////////////////
//     Function: CycleDataStageReader::operator -> (trivial)
//       Access: Public
//  Description: This provides an indirect member access to the actual
//               CycleData data.
////////////////////////////////////////////////////////////////////
template<class CycleDataType>
INLINE const CycleDataType *CycleDataStageReader<CycleDataType>::
operator -> () const {
  return _pointer;
}

////////////////////////////////////////////////////////////////////
//     Function: CycleDataStageReader::Typecast pointer (trivial)
//       Access: Public
//  Description: This allows the CycleDataStageReader to be passed to any
//               function that expects a const CycleDataType pointer.
////////////////////////////////////////////////////////////////////
template<class CycleDataType>
INLINE CycleDataStageReader<CycleDataType>::
operator const CycleDataType * () const {
  return _pointer;
}

////////////////////////////////////////////////////////////////////
//     Function: CycleDataStageReader::get_current_thread (trivial)
//       Access: Public
//  Description: Returns the Thread pointer of the currently-executing
//               thread, as passed to the constructor of this object.
////////////////////////////////////////////////////////////////////
template<class CycleDataType>
INLINE Thread *CycleDataStageReader<CycleDataType>::
get_current_thread() const {
  return Thread::get_current_thread();
}

#endif  // DO_PIPELINING
#endif  // CPPPARSER
