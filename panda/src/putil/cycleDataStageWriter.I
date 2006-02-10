// Filename: cycleDataStageWriter.I
// Created by:  drose (06Feb06)
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
//     Function: CycleDataStageWriter::Constructor (full)
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
template<class CycleDataType>
INLINE CycleDataStageWriter<CycleDataType>::
CycleDataStageWriter(PipelineCycler<CycleDataType> &cycler, int stage) :
  _cycler(&cycler),
  _stage(stage)
{
  _pointer = _cycler->write_stage(_stage);
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
  _pointer(copy._pointer),
  _stage(copy._stage)
{
  nassertv(_pointer != (CycleDataType *)NULL);
  _cycler->increment_write(_pointer);
}

////////////////////////////////////////////////////////////////////
//     Function: CycleDataStageWriter::Copy Assigment (full)
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
template<class CycleDataType>
INLINE void CycleDataStageWriter<CycleDataType>::
operator = (const CycleDataStageWriter<CycleDataType> &copy) {
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
//               from the CycleDataStageReader from a read to a write
//               pointer (and invalidates the reader).
////////////////////////////////////////////////////////////////////
template<class CycleDataType>
INLINE CycleDataStageWriter<CycleDataType>::
CycleDataStageWriter(PipelineCycler<CycleDataType> &cycler, int stage,
                     CycleDataStageReader<CycleDataType> &take_from) :
  _cycler(&cycler),
  _stage(stage)
{
  _pointer = _cycler->elevate_read_stage(_stage, take_from.take_pointer());
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

#else  // !DO_PIPELINING
// This is the trivial, do-nothing implementation.

////////////////////////////////////////////////////////////////////
//     Function: CycleDataStageWriter::Constructor (trivial)
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
template<class CycleDataType>
INLINE CycleDataStageWriter<CycleDataType>::
CycleDataStageWriter(PipelineCycler<CycleDataType> &cycler, int) {
  _pointer = cycler.write();
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
//     Function: CycleDataStageWriter::Copy Assigment (trivial)
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
//               from the CycleDataStageReader from a read to a write
//               pointer (and invalidates the reader).
////////////////////////////////////////////////////////////////////
template<class CycleDataType>
INLINE CycleDataStageWriter<CycleDataType>::
CycleDataStageWriter(PipelineCycler<CycleDataType> &, int,
                     CycleDataStageReader<CycleDataType> &take_from) :
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

#endif  // DO_PIPELINING
