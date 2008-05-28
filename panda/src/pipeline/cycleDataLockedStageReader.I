// Filename: cycleDataLockedStageReader.I
// Created by:  drose (30Apr06)
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
//     Function: CycleDataLockedStageReader::Constructor (full)
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
template<class CycleDataType>
INLINE CycleDataLockedStageReader<CycleDataType>::
CycleDataLockedStageReader(const PipelineCycler<CycleDataType> &cycler,
                           int stage, Thread *current_thread) :
  _cycler(&cycler),
  _current_thread(current_thread),
  _stage(stage)
{
  _pointer = _cycler->read_stage(_stage, _current_thread);
  nassertv(_pointer != (const CycleDataType *)NULL);
}

////////////////////////////////////////////////////////////////////
//     Function: CycleDataLockedStageReader::Copy Constructor (full)
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
template<class CycleDataType>
INLINE CycleDataLockedStageReader<CycleDataType>::
CycleDataLockedStageReader(const CycleDataLockedStageReader<CycleDataType> &copy) :
  _cycler(copy._cycler),
  _current_thread(copy._current_thread),
  _pointer(copy._pointer),
  _stage(copy._stage)
{
  nassertv(_pointer != (const CycleDataType *)NULL);
  _cycler->increment_read(_pointer);
}

////////////////////////////////////////////////////////////////////
//     Function: CycleDataLockedStageReader::Copy Assignment (full)
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
template<class CycleDataType>
INLINE void CycleDataLockedStageReader<CycleDataType>::
operator = (const CycleDataLockedStageReader<CycleDataType> &copy) {
  nassertv(_pointer == (CycleDataType *)NULL);
  nassertv(_current_thread == copy._current_thread);

  _cycler = copy._cycler;
  _pointer = copy._pointer;
  _stage = copy._stage;

  nassertv(_pointer != (const CycleDataType *)NULL);
  _cycler->increment_read(_pointer);
}

////////////////////////////////////////////////////////////////////
//     Function: CycleDataLockedStageReader::Destructor (full)
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
template<class CycleDataType>
INLINE CycleDataLockedStageReader<CycleDataType>::
~CycleDataLockedStageReader() {
  if (_pointer != NULL) {
    _cycler->release_read_stage(_stage, _pointer);
  }
}

////////////////////////////////////////////////////////////////////
//     Function: CycleDataLockedStageReader::operator -> (full)
//       Access: Public
//  Description: This provides an indirect member access to the actual
//               CycleData data.
////////////////////////////////////////////////////////////////////
template<class CycleDataType>
INLINE const CycleDataType *CycleDataLockedStageReader<CycleDataType>::
operator -> () const {
  nassertr(_pointer != (const CycleDataType *)NULL, _cycler->cheat());
  return _pointer;
}

////////////////////////////////////////////////////////////////////
//     Function: CycleDataLockedStageReader::Typecast pointer (full)
//       Access: Public
//  Description: This allows the CycleDataLockedStageReader to be passed to any
//               function that expects a const CycleDataType pointer.
////////////////////////////////////////////////////////////////////
template<class CycleDataType>
INLINE CycleDataLockedStageReader<CycleDataType>::
operator const CycleDataType * () const {
  nassertr(_pointer != (const CycleDataType *)NULL, _cycler->cheat());
  return _pointer;
}

////////////////////////////////////////////////////////////////////
//     Function: CycleDataLockedStageReader::take_pointer (full)
//       Access: Public
//  Description: This is intended to be called only from
//               CycleDataStageWriter when it elevates the pointer from
//               read to write status.  This function returns the
//               reader's pointer and relinquishes ownership of the
//               pointer, rendering the reader invalid for future
//               reads.
////////////////////////////////////////////////////////////////////
template<class CycleDataType>
INLINE const CycleDataType *CycleDataLockedStageReader<CycleDataType>::
take_pointer() {
  const CycleDataType *pointer = _pointer;
  _pointer = (CycleDataType *)NULL;
  nassertr(pointer != (const CycleDataType *)NULL, _cycler->cheat());
  return pointer;
}

////////////////////////////////////////////////////////////////////
//     Function: CycleDataLockedStageReader::get_current_thread (full)
//       Access: Public
//  Description: Returns the Thread pointer of the currently-executing
//               thread, as passed to the constructor of this object.
////////////////////////////////////////////////////////////////////
template<class CycleDataType>
INLINE Thread *CycleDataLockedStageReader<CycleDataType>::
get_current_thread() const {
  return _current_thread;
}

#else  // !DO_PIPELINING
// This is the trivial, do-nothing implementation.

////////////////////////////////////////////////////////////////////
//     Function: CycleDataLockedStageReader::Constructor (trivial)
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
template<class CycleDataType>
INLINE CycleDataLockedStageReader<CycleDataType>::
CycleDataLockedStageReader(const PipelineCycler<CycleDataType> &cycler, int,
                           Thread *) {
  _pointer = cycler.cheat();
}

////////////////////////////////////////////////////////////////////
//     Function: CycleDataLockedStageReader::Copy Constructor (trivial)
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
template<class CycleDataType>
INLINE CycleDataLockedStageReader<CycleDataType>::
CycleDataLockedStageReader(const CycleDataLockedStageReader<CycleDataType> &copy) :
  _pointer(copy._pointer)
{
}

////////////////////////////////////////////////////////////////////
//     Function: CycleDataLockedStageReader::Copy Assignment (trivial)
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
template<class CycleDataType>
INLINE void CycleDataLockedStageReader<CycleDataType>::
operator = (const CycleDataLockedStageReader<CycleDataType> &copy) {
  _pointer = copy._pointer;
}

////////////////////////////////////////////////////////////////////
//     Function: CycleDataLockedStageReader::Destructor (trivial)
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
template<class CycleDataType>
INLINE CycleDataLockedStageReader<CycleDataType>::
~CycleDataLockedStageReader() {
}

////////////////////////////////////////////////////////////////////
//     Function: CycleDataLockedStageReader::operator -> (trivial)
//       Access: Public
//  Description: This provides an indirect member access to the actual
//               CycleData data.
////////////////////////////////////////////////////////////////////
template<class CycleDataType>
INLINE const CycleDataType *CycleDataLockedStageReader<CycleDataType>::
operator -> () const {
  return _pointer;
}

////////////////////////////////////////////////////////////////////
//     Function: CycleDataLockedStageReader::Typecast pointer (trivial)
//       Access: Public
//  Description: This allows the CycleDataLockedStageReader to be passed to any
//               function that expects a const CycleDataType pointer.
////////////////////////////////////////////////////////////////////
template<class CycleDataType>
INLINE CycleDataLockedStageReader<CycleDataType>::
operator const CycleDataType * () const {
  return _pointer;
}

////////////////////////////////////////////////////////////////////
//     Function: CycleDataLockedStageReader::take_pointer (trivial)
//       Access: Public
//  Description: This is intended to be called only from
//               CycleDataStageWriter when it elevates the pointer from
//               read to write status.  This function returns the
//               reader's pointer and relinquishes ownership of the
//               pointer, rendering the reader invalid for future
//               reads.
////////////////////////////////////////////////////////////////////
template<class CycleDataType>
INLINE const CycleDataType *CycleDataLockedStageReader<CycleDataType>::
take_pointer() {
  return _pointer;
}

////////////////////////////////////////////////////////////////////
//     Function: CycleDataLockedStageReader::get_current_thread (trivial)
//       Access: Public
//  Description: Returns the Thread pointer of the currently-executing
//               thread, as passed to the constructor of this object.
////////////////////////////////////////////////////////////////////
template<class CycleDataType>
INLINE Thread *CycleDataLockedStageReader<CycleDataType>::
get_current_thread() const {
  return Thread::get_current_thread();
}

#endif  // DO_PIPELINING
#endif  // CPPPARSER
