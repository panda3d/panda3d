// Filename: cycleDataStageWriter.I
// Created by:  drose (06Feb06)
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
//     Function: CycleDataStageWriter::Constructor (full)
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
template<class CycleDataType>
INLINE CycleDataStageWriter<CycleDataType>::
CycleDataStageWriter(PipelineCycler<CycleDataType> &cycler, int stage,
                     Thread *current_thread) :
  _cycler(&cycler),
  _current_thread(current_thread),
  _stage(stage)
{
  _pointer = _cycler->write_stage(_stage, _current_thread);
  nassertv(_pointer != (CycleDataType *)NULL);
}

////////////////////////////////////////////////////////////////////
//     Function: CycleDataStageWriter::Constructor (full)
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
template<class CycleDataType>
INLINE CycleDataStageWriter<CycleDataType>::
CycleDataStageWriter(PipelineCycler<CycleDataType> &cycler, int stage,
                     bool force_to_0, Thread *current_thread) :
  _cycler(&cycler),
  _current_thread(current_thread),
  _stage(stage)
{
  _pointer = _cycler->write_stage_upstream(_stage, force_to_0, _current_thread);
  nassertv(_pointer != (CycleDataType *)NULL);
}

////////////////////////////////////////////////////////////////////
//     Function: CycleDataStageWriter::Copy Constructor (full)
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
template<class CycleDataType>
INLINE CycleDataStageWriter<CycleDataType>::
CycleDataStageWriter(const CycleDataStageWriter<CycleDataType> &copy) :
  _cycler(copy._cycler),
  _current_thread(copy._current_thread),
  _pointer(copy._pointer),
  _stage(copy._stage)
{
  nassertv(_pointer != (CycleDataType *)NULL);
  _cycler->increment_write(_pointer);
}

////////////////////////////////////////////////////////////////////
//     Function: CycleDataStageWriter::Copy Assignment (full)
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
template<class CycleDataType>
INLINE void CycleDataStageWriter<CycleDataType>::
operator = (const CycleDataStageWriter<CycleDataType> &copy) {
  nassertv(_pointer == (CycleDataType *)NULL);
  nassertv(_current_thread == copy._current_thread);

  _cycler = copy._cycler;
  _pointer = copy._pointer;
  _stage = copy._stage;

  nassertv(_pointer != (CycleDataType *)NULL);
  _cycler->increment_write(_pointer);
}

////////////////////////////////////////////////////////////////////
//     Function: CycleDataStageWriter::Constructor (full)
//       Access: Public
//  Description: This flavor of the constructor elevates the pointer
//               from the CycleDataLockedStageReader from a read to a write
//               pointer (and invalidates the reader).
////////////////////////////////////////////////////////////////////
template<class CycleDataType>
INLINE CycleDataStageWriter<CycleDataType>::
CycleDataStageWriter(PipelineCycler<CycleDataType> &cycler, int stage,
                     CycleDataLockedStageReader<CycleDataType> &take_from) :
  _cycler(&cycler),
  _current_thread(take_from.get_current_thread()),
  _stage(stage)
{
  _pointer = _cycler->elevate_read_stage(_stage, take_from.take_pointer(),
                                         _current_thread);
}

////////////////////////////////////////////////////////////////////
//     Function: CycleDataStageWriter::Constructor (full)
//       Access: Public
//  Description: This flavor of the constructor elevates the pointer
//               from the CycleDataLockedStageReader from a read to a write
//               pointer (and invalidates the reader).
////////////////////////////////////////////////////////////////////
template<class CycleDataType>
INLINE CycleDataStageWriter<CycleDataType>::
CycleDataStageWriter(PipelineCycler<CycleDataType> &cycler, int stage,
                     CycleDataLockedStageReader<CycleDataType> &take_from,
                     bool force_to_0) :
  _cycler(&cycler),
  _current_thread(take_from.get_current_thread()),
  _stage(stage)
{
  _pointer = _cycler->elevate_read_stage_upstream(_stage, take_from.take_pointer(),
                                                  force_to_0, _current_thread);
}

////////////////////////////////////////////////////////////////////
//     Function: CycleDataStageWriter::Destructor (full)
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
template<class CycleDataType>
INLINE CycleDataStageWriter<CycleDataType>::
~CycleDataStageWriter() {
  if (_pointer != (CycleDataType *)NULL) {
    _cycler->release_write_stage(_stage, _pointer);
  }
}

////////////////////////////////////////////////////////////////////
//     Function: CycleDataStageWriter::operator -> (full)
//       Access: Public
//  Description: This provides an indirect member access to the actual
//               CycleData data.
////////////////////////////////////////////////////////////////////
template<class CycleDataType>
INLINE CycleDataType *CycleDataStageWriter<CycleDataType>::
operator -> () {
  nassertr(_pointer != (CycleDataType *)NULL, _cycler->cheat());
  return _pointer;
}

////////////////////////////////////////////////////////////////////
//     Function: CycleDataStageWriter::operator -> (full)
//       Access: Public
//  Description: This provides an indirect member access to the actual
//               CycleData data.
////////////////////////////////////////////////////////////////////
template<class CycleDataType>
INLINE const CycleDataType *CycleDataStageWriter<CycleDataType>::
operator -> () const {
  nassertr(_pointer != (CycleDataType *)NULL, _cycler->cheat());
  return _pointer;
}

////////////////////////////////////////////////////////////////////
//     Function: CycleDataStageWriter::Typecast pointer (full)
//       Access: Public
//  Description: This allows the CycleDataStageWriter to be passed to any
//               function that expects a CycleDataType pointer.
////////////////////////////////////////////////////////////////////
template<class CycleDataType>
INLINE CycleDataStageWriter<CycleDataType>::
operator CycleDataType * () {
  nassertr(_pointer != (CycleDataType *)NULL, _cycler->cheat());
  return _pointer;
}

////////////////////////////////////////////////////////////////////
//     Function: CycleDataStageWriter::get_current_thread (full)
//       Access: Public
//  Description: Returns the Thread pointer of the currently-executing
//               thread, as passed to the constructor of this object.
////////////////////////////////////////////////////////////////////
template<class CycleDataType>
INLINE Thread *CycleDataStageWriter<CycleDataType>::
get_current_thread() const {
  return _current_thread;
}

#else  // !DO_PIPELINING
// This is the trivial, do-nothing implementation.

////////////////////////////////////////////////////////////////////
//     Function: CycleDataStageWriter::Constructor (trivial)
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
template<class CycleDataType>
INLINE CycleDataStageWriter<CycleDataType>::
CycleDataStageWriter(PipelineCycler<CycleDataType> &cycler, int, Thread *) {
  _pointer = cycler.cheat();
}

////////////////////////////////////////////////////////////////////
//     Function: CycleDataStageWriter::Constructor (trivial)
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
template<class CycleDataType>
INLINE CycleDataStageWriter<CycleDataType>::
CycleDataStageWriter(PipelineCycler<CycleDataType> &cycler, int, bool, Thread *) {
  _pointer = cycler.cheat();
}

////////////////////////////////////////////////////////////////////
//     Function: CycleDataStageWriter::Copy Constructor (trivial)
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
template<class CycleDataType>
INLINE CycleDataStageWriter<CycleDataType>::
CycleDataStageWriter(const CycleDataStageWriter<CycleDataType> &copy) :
  _pointer(copy._pointer)
{
}

////////////////////////////////////////////////////////////////////
//     Function: CycleDataStageWriter::Copy Assignment (trivial)
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
template<class CycleDataType>
INLINE void CycleDataStageWriter<CycleDataType>::
operator = (const CycleDataStageWriter<CycleDataType> &copy) {
  _pointer = copy._pointer;
}

////////////////////////////////////////////////////////////////////
//     Function: CycleDataStageWriter::Constructor (trivial)
//       Access: Public
//  Description: This flavor of the constructor elevates the pointer
//               from the CycleDataLockedStageReader from a read to a write
//               pointer (and invalidates the reader).
////////////////////////////////////////////////////////////////////
template<class CycleDataType>
INLINE CycleDataStageWriter<CycleDataType>::
CycleDataStageWriter(PipelineCycler<CycleDataType> &, int,
                     CycleDataLockedStageReader<CycleDataType> &take_from) :
  _pointer((CycleDataType *)take_from.take_pointer())
{
}

////////////////////////////////////////////////////////////////////
//     Function: CycleDataStageWriter::Constructor (trivial)
//       Access: Public
//  Description: This flavor of the constructor elevates the pointer
//               from the CycleDataLockedStageReader from a read to a write
//               pointer (and invalidates the reader).
////////////////////////////////////////////////////////////////////
template<class CycleDataType>
INLINE CycleDataStageWriter<CycleDataType>::
CycleDataStageWriter(PipelineCycler<CycleDataType> &, int,
                     CycleDataLockedStageReader<CycleDataType> &take_from,
                     bool) :
  _pointer((CycleDataType *)take_from.take_pointer())
{
}

////////////////////////////////////////////////////////////////////
//     Function: CycleDataStageWriter::Destructor (trivial)
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
template<class CycleDataType>
INLINE CycleDataStageWriter<CycleDataType>::
~CycleDataStageWriter() {
}

////////////////////////////////////////////////////////////////////
//     Function: CycleDataStageWriter::operator -> (trivial)
//       Access: Public
//  Description: This provides an indirect member access to the actual
//               CycleData data.
////////////////////////////////////////////////////////////////////
template<class CycleDataType>
INLINE CycleDataType *CycleDataStageWriter<CycleDataType>::
operator -> () {
  return _pointer;
}

////////////////////////////////////////////////////////////////////
//     Function: CycleDataStageWriter::operator -> (trivial)
//       Access: Public
//  Description: This provides an indirect member access to the actual
//               CycleData data.
////////////////////////////////////////////////////////////////////
template<class CycleDataType>
INLINE const CycleDataType *CycleDataStageWriter<CycleDataType>::
operator -> () const {
  return _pointer;
}

////////////////////////////////////////////////////////////////////
//     Function: CycleDataStageWriter::Typecast pointer (trivial)
//       Access: Public
//  Description: This allows the CycleDataStageWriter to be passed to any
//               function that expects a CycleDataType pointer.
////////////////////////////////////////////////////////////////////
template<class CycleDataType>
INLINE CycleDataStageWriter<CycleDataType>::
operator CycleDataType * () {
  return _pointer;
}

////////////////////////////////////////////////////////////////////
//     Function: CycleDataStageWriter::get_current_thread (trivial)
//       Access: Public
//  Description: Returns the Thread pointer of the currently-executing
//               thread, as passed to the constructor of this object.
////////////////////////////////////////////////////////////////////
template<class CycleDataType>
INLINE Thread *CycleDataStageWriter<CycleDataType>::
get_current_thread() const {
  return Thread::get_current_thread();
}

#endif  // DO_PIPELINING
#endif  // CPPPARSER
