// Filename: cycleDataReader.I
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
// This is the implementation for full support of pipelining (as well
// as the sanity-check only implementation).

////////////////////////////////////////////////////////////////////
//     Function: CycleDataReader::Constructor (full)
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
template<class CycleDataType>
INLINE CycleDataReader<CycleDataType>::
CycleDataReader(const PipelineCycler<CycleDataType> &cycler,
                Thread *current_thread) :
  _cycler(&cycler),
  _current_thread(current_thread)
{
  _pointer = _cycler->read(_current_thread);
  nassertv(_pointer != (const CycleDataType *)NULL);
}

////////////////////////////////////////////////////////////////////
//     Function: CycleDataReader::Copy Constructor (full)
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
template<class CycleDataType>
INLINE CycleDataReader<CycleDataType>::
CycleDataReader(const CycleDataReader<CycleDataType> &copy) :
  _cycler(copy._cycler),
  _current_thread(copy._current_thread),
  _pointer(copy._pointer)
{
  nassertv(_pointer != (const CycleDataType *)NULL);
  _cycler->increment_read(_pointer);
}

////////////////////////////////////////////////////////////////////
//     Function: CycleDataReader::Copy Assignment (full)
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
template<class CycleDataType>
INLINE void CycleDataReader<CycleDataType>::
operator = (const CycleDataReader<CycleDataType> &copy) {
  nassertv(_pointer == (CycleDataType *)NULL);
  nassertv(_current_thread == copy._current_thread);

  _cycler = copy._cycler;
  _pointer = copy._pointer;

  nassertv(_pointer != (const CycleDataType *)NULL);
  _cycler->increment_read(_pointer);
}

////////////////////////////////////////////////////////////////////
//     Function: CycleDataReader::Destructor (full)
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
template<class CycleDataType>
INLINE CycleDataReader<CycleDataType>::
~CycleDataReader() {
  if (_pointer != NULL) {
    _cycler->release_read(_pointer);
  }
}

////////////////////////////////////////////////////////////////////
//     Function: CycleDataReader::operator -> (full)
//       Access: Public
//  Description: This provides an indirect member access to the actual
//               CycleData data.
////////////////////////////////////////////////////////////////////
template<class CycleDataType>
INLINE const CycleDataType *CycleDataReader<CycleDataType>::
operator -> () const {
  nassertr(_pointer != (const CycleDataType *)NULL, _cycler->cheat());
  return _pointer;
}

////////////////////////////////////////////////////////////////////
//     Function: CycleDataReader::Typecast pointer (full)
//       Access: Public
//  Description: This allows the CycleDataReader to be passed to any
//               function that expects a const CycleDataType pointer.
////////////////////////////////////////////////////////////////////
template<class CycleDataType>
INLINE CycleDataReader<CycleDataType>::
operator const CycleDataType * () const {
  nassertr(_pointer != (const CycleDataType *)NULL, _cycler->cheat());
  return _pointer;
}

////////////////////////////////////////////////////////////////////
//     Function: CycleDataReader::take_pointer (full)
//       Access: Public
//  Description: This is intended to be called only from
//               CycleDataWriter when it elevates the pointer from
//               read to write status.  This function returns the
//               reader's pointer and relinquishes ownership of the
//               pointer, rendering the reader invalid for future
//               reads.
////////////////////////////////////////////////////////////////////
template<class CycleDataType>
INLINE const CycleDataType *CycleDataReader<CycleDataType>::
take_pointer() {
  const CycleDataType *pointer = _pointer;
  _pointer = (CycleDataType *)NULL;
  nassertr(pointer != (const CycleDataType *)NULL, _cycler->cheat());
  return pointer;
}

////////////////////////////////////////////////////////////////////
//     Function: CycleDataReader::get_current_thread (full)
//       Access: Public
//  Description: Returns the Thread pointer of the currently-executing
//               thread, as passed to the constructor of this object.
////////////////////////////////////////////////////////////////////
template<class CycleDataType>
INLINE Thread *CycleDataReader<CycleDataType>::
get_current_thread() const {
  return _current_thread;
}

#else  // !DO_PIPELINING
// This is the trivial, do-nothing implementation.

////////////////////////////////////////////////////////////////////
//     Function: CycleDataReader::Constructor (trivial)
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
template<class CycleDataType>
INLINE CycleDataReader<CycleDataType>::
CycleDataReader(const PipelineCycler<CycleDataType> &cycler, Thread *) {
  _pointer = cycler.cheat();
}

////////////////////////////////////////////////////////////////////
//     Function: CycleDataReader::Copy Constructor (trivial)
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
template<class CycleDataType>
INLINE CycleDataReader<CycleDataType>::
CycleDataReader(const CycleDataReader<CycleDataType> &copy) :
  _pointer(copy._pointer)
{
}

////////////////////////////////////////////////////////////////////
//     Function: CycleDataReader::Copy Assignment (trivial)
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
template<class CycleDataType>
INLINE void CycleDataReader<CycleDataType>::
operator = (const CycleDataReader<CycleDataType> &copy) {
  _pointer = copy._pointer;
}

////////////////////////////////////////////////////////////////////
//     Function: CycleDataReader::Destructor (trivial)
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
template<class CycleDataType>
INLINE CycleDataReader<CycleDataType>::
~CycleDataReader() {
}

////////////////////////////////////////////////////////////////////
//     Function: CycleDataReader::operator -> (trivial)
//       Access: Public
//  Description: This provides an indirect member access to the actual
//               CycleData data.
////////////////////////////////////////////////////////////////////
template<class CycleDataType>
INLINE const CycleDataType *CycleDataReader<CycleDataType>::
operator -> () const {
  return _pointer;
}

////////////////////////////////////////////////////////////////////
//     Function: CycleDataReader::Typecast pointer (trivial)
//       Access: Public
//  Description: This allows the CycleDataReader to be passed to any
//               function that expects a const CycleDataType pointer.
////////////////////////////////////////////////////////////////////
template<class CycleDataType>
INLINE CycleDataReader<CycleDataType>::
operator const CycleDataType * () const {
  return _pointer;
}

////////////////////////////////////////////////////////////////////
//     Function: CycleDataReader::take_pointer (trivial)
//       Access: Public
//  Description: This is intended to be called only from
//               CycleDataWriter when it elevates the pointer from
//               read to write status.  This function returns the
//               reader's pointer and relinquishes ownership of the
//               pointer, rendering the reader invalid for future
//               reads.
////////////////////////////////////////////////////////////////////
template<class CycleDataType>
INLINE const CycleDataType *CycleDataReader<CycleDataType>::
take_pointer() {
  return _pointer;
}

////////////////////////////////////////////////////////////////////
//     Function: CycleDataReader::get_current_thread (trivial)
//       Access: Public
//  Description: Returns the Thread pointer of the currently-executing
//               thread, as passed to the constructor of this object.
////////////////////////////////////////////////////////////////////
template<class CycleDataType>
INLINE Thread *CycleDataReader<CycleDataType>::
get_current_thread() const {
  return Thread::get_current_thread();
}

#endif  // DO_PIPELINING
