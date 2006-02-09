// Filename: cycleDataStageReader.I
// Created by:  drose (08Feb06)
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
//     Function: CycleDataStageReader::Constructor (full)
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
template<class CycleDataType>
INLINE CycleDataStageReader<CycleDataType>::
CycleDataStageReader(const PipelineCycler<CycleDataType> &cycler,
                     int stage) :
  _cycler(&cycler),
  _stage(stage)
{
  _pointer = _cycler->read_stage(_stage);
  nassertv(_pointer != (const CycleDataType *)NULL);
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
  _pointer(copy._pointer),
  _stage(copy._stage)
{
  nassertv(_pointer != (const CycleDataType *)NULL);
  _cycler->increment_read(_pointer);
}

////////////////////////////////////////////////////////////////////
//     Function: CycleDataStageReader::Copy Assignment (full)
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
template<class CycleDataType>
INLINE void CycleDataStageReader<CycleDataType>::
operator = (const CycleDataStageReader<CycleDataType> &copy) {
  _cycler = copy._cycler;
  _pointer = copy._pointer;
  _stage = copy._stage;

  nassertv(_pointer != (const CycleDataType *)NULL);
  _cycler->increment_read(_pointer);
}

////////////////////////////////////////////////////////////////////
//     Function: CycleDataStageReader::Destructor (full)
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
template<class CycleDataType>
INLINE CycleDataStageReader<CycleDataType>::
~CycleDataStageReader() {
  if (_pointer != NULL) {
    _cycler->release_read_stage(_stage, _pointer);
  }
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
  nassertr(_pointer != (const CycleDataType *)NULL, _cycler->cheat());
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
  nassertr(_pointer != (const CycleDataType *)NULL, _cycler->cheat());
  return _pointer;
}

////////////////////////////////////////////////////////////////////
//     Function: CycleDataStageReader::take_pointer (full)
//       Access: Public
//  Description: This is intended to be called only from
//               CycleDataStageWriter when it elevates the pointer from
//               read to write status.  This function returns the
//               reader's pointer and relinquishes ownership of the
//               pointer, rendering the reader invalid for future
//               reads.
////////////////////////////////////////////////////////////////////
template<class CycleDataType>
INLINE const CycleDataType *CycleDataStageReader<CycleDataType>::
take_pointer() {
  const CycleDataType *pointer = _pointer;
  _pointer = (CycleDataType *)NULL;
  nassertr(pointer != (const CycleDataType *)NULL, _cycler->cheat());
  return pointer;
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
CycleDataStageReader(const PipelineCycler<CycleDataType> &cycler, int) {
  _pointer = cycler.read();
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
//     Function: CycleDataStageReader::take_pointer (trivial)
//       Access: Public
//  Description: This is intended to be called only from
//               CycleDataStageWriter when it elevates the pointer from
//               read to write status.  This function returns the
//               reader's pointer and relinquishes ownership of the
//               pointer, rendering the reader invalid for future
//               reads.
////////////////////////////////////////////////////////////////////
template<class CycleDataType>
INLINE const CycleDataType *CycleDataStageReader<CycleDataType>::
take_pointer() {
  return _pointer;
}

#endif  // DO_PIPELINING
