// Filename: cycleDataLockedReader.I
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
//     Function: CycleDataLockedReader::Constructor (full)
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
template<class CycleDataType>
INLINE CycleDataLockedReader<CycleDataType>::
CycleDataLockedReader(const PipelineCycler<CycleDataType> &cycler,
                      Thread *current_thread) :
  _cycler(&cycler),
  _current_thread(current_thread)
{
  _pointer = _cycler->read(_current_thread);
  nassertv(_pointer != (const CycleDataType *)NULL);
}

////////////////////////////////////////////////////////////////////
//     Function: CycleDataLockedReader::Copy Constructor (full)
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
template<class CycleDataType>
INLINE CycleDataLockedReader<CycleDataType>::
CycleDataLockedReader(const CycleDataLockedReader<CycleDataType> &copy) :
  _cycler(copy._cycler),
  _current_thread(copy._current_thread),
  _pointer(copy._pointer)
{
  nassertv(_pointer != (const CycleDataType *)NULL);
  _cycler->increment_read(_pointer);
}

////////////////////////////////////////////////////////////////////
//     Function: CycleDataLockedReader::Copy Assignment (full)
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
template<class CycleDataType>
INLINE void CycleDataLockedReader<CycleDataType>::
operator = (const CycleDataLockedReader<CycleDataType> &copy) {
  nassertv(_pointer == (CycleDataType *)NULL);
  nassertv(_current_thread == copy._current_thread);

  _cycler = copy._cycler;
  _pointer = copy._pointer;

  nassertv(_pointer != (const CycleDataType *)NULL);
  _cycler->increment_read(_pointer);
}

////////////////////////////////////////////////////////////////////
//     Function: CycleDataLockedReader::Destructor (full)
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
template<class CycleDataType>
INLINE CycleDataLockedReader<CycleDataType>::
~CycleDataLockedReader() {
  if (_pointer != NULL) {
    _cycler->release_read(_pointer);
  }
}

////////////////////////////////////////////////////////////////////
//     Function: CycleDataLockedReader::operator -> (full)
//       Access: Public
//  Description: This provides an indirect member access to the actual
//               CycleData data.
////////////////////////////////////////////////////////////////////
template<class CycleDataType>
INLINE const CycleDataType *CycleDataLockedReader<CycleDataType>::
operator -> () const {
  nassertr(_pointer != (const CycleDataType *)NULL, _cycler->cheat());
  return _pointer;
}

////////////////////////////////////////////////////////////////////
//     Function: CycleDataLockedReader::Typecast pointer (full)
//       Access: Public
//  Description: This allows the CycleDataLockedReader to be passed to any
//               function that expects a const CycleDataType pointer.
////////////////////////////////////////////////////////////////////
template<class CycleDataType>
INLINE CycleDataLockedReader<CycleDataType>::
operator const CycleDataType * () const {
  nassertr(_pointer != (const CycleDataType *)NULL, _cycler->cheat());
  return _pointer;
}

////////////////////////////////////////////////////////////////////
//     Function: CycleDataLockedReader::take_pointer (full)
//       Access: Public
//  Description: This is intended to be called only from
//               CycleDataWriter when it elevates the pointer from
//               read to write status.  This function returns the
//               reader's pointer and relinquishes ownership of the
//               pointer, rendering the reader invalid for future
//               reads.
////////////////////////////////////////////////////////////////////
template<class CycleDataType>
INLINE const CycleDataType *CycleDataLockedReader<CycleDataType>::
take_pointer() {
  const CycleDataType *pointer = _pointer;
  _pointer = (CycleDataType *)NULL;
  nassertr(pointer != (const CycleDataType *)NULL, _cycler->cheat());
  return pointer;
}

////////////////////////////////////////////////////////////////////
//     Function: CycleDataLockedReader::get_current_thread (full)
//       Access: Public
//  Description: Returns the Thread pointer of the currently-executing
//               thread, as passed to the constructor of this object.
////////////////////////////////////////////////////////////////////
template<class CycleDataType>
INLINE Thread *CycleDataLockedReader<CycleDataType>::
get_current_thread() const {
  return _current_thread;
}

#else  // !DO_PIPELINING
// This is the trivial, do-nothing implementation.

////////////////////////////////////////////////////////////////////
//     Function: CycleDataLockedReader::Constructor (trivial)
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
template<class CycleDataType>
INLINE CycleDataLockedReader<CycleDataType>::
CycleDataLockedReader(const PipelineCycler<CycleDataType> &cycler, Thread *) {
  _pointer = cycler.cheat();
}

////////////////////////////////////////////////////////////////////
//     Function: CycleDataLockedReader::Copy Constructor (trivial)
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
template<class CycleDataType>
INLINE CycleDataLockedReader<CycleDataType>::
CycleDataLockedReader(const CycleDataLockedReader<CycleDataType> &copy) :
  _pointer(copy._pointer)
{
}

////////////////////////////////////////////////////////////////////
//     Function: CycleDataLockedReader::Copy Assignment (trivial)
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
template<class CycleDataType>
INLINE void CycleDataLockedReader<CycleDataType>::
operator = (const CycleDataLockedReader<CycleDataType> &copy) {
  _pointer = copy._pointer;
}

////////////////////////////////////////////////////////////////////
//     Function: CycleDataLockedReader::Destructor (trivial)
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
template<class CycleDataType>
INLINE CycleDataLockedReader<CycleDataType>::
~CycleDataLockedReader() {
}

////////////////////////////////////////////////////////////////////
//     Function: CycleDataLockedReader::operator -> (trivial)
//       Access: Public
//  Description: This provides an indirect member access to the actual
//               CycleData data.
////////////////////////////////////////////////////////////////////
template<class CycleDataType>
INLINE const CycleDataType *CycleDataLockedReader<CycleDataType>::
operator -> () const {
  return _pointer;
}

////////////////////////////////////////////////////////////////////
//     Function: CycleDataLockedReader::Typecast pointer (trivial)
//       Access: Public
//  Description: This allows the CycleDataLockedReader to be passed to any
//               function that expects a const CycleDataType pointer.
////////////////////////////////////////////////////////////////////
template<class CycleDataType>
INLINE CycleDataLockedReader<CycleDataType>::
operator const CycleDataType * () const {
  return _pointer;
}

////////////////////////////////////////////////////////////////////
//     Function: CycleDataLockedReader::take_pointer (trivial)
//       Access: Public
//  Description: This is intended to be called only from
//               CycleDataWriter when it elevates the pointer from
//               read to write status.  This function returns the
//               reader's pointer and relinquishes ownership of the
//               pointer, rendering the reader invalid for future
//               reads.
////////////////////////////////////////////////////////////////////
template<class CycleDataType>
INLINE const CycleDataType *CycleDataLockedReader<CycleDataType>::
take_pointer() {
  return _pointer;
}

////////////////////////////////////////////////////////////////////
//     Function: CycleDataLockedReader::get_current_thread (trivial)
//       Access: Public
//  Description: Returns the Thread pointer of the currently-executing
//               thread, as passed to the constructor of this object.
////////////////////////////////////////////////////////////////////
template<class CycleDataType>
INLINE Thread *CycleDataLockedReader<CycleDataType>::
get_current_thread() const {
  return Thread::get_current_thread();
}

#endif  // DO_PIPELINING
#endif  // CPPPARSER
