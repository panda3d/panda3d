// Filename: cycleDataWriter.I
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
//     Function: CycleDataWriter::Constructor
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
template<class CycleDataType>
INLINE CycleDataWriter<CycleDataType>::
CycleDataWriter(PipelineCycler<CycleDataType> &cycler) :
  _cycler(cycler)
{
  _pointer = _cycler.write();
}

////////////////////////////////////////////////////////////////////
//     Function: CycleDataWriter::Constructor
//       Access: Public
//  Description: This is a lot like a copy constructor, in that the
//               new CycleDataWriter object gets a handle to the same
//               pointer held by the old CycleDataWriter object.
//               However, since only one write pointer may be active
//               at a time, this invalidates the old object.
////////////////////////////////////////////////////////////////////
template<class CycleDataType>
INLINE CycleDataWriter<CycleDataType>::
CycleDataWriter(PipelineCycler<CycleDataType> &cycler, 
                CycleDataWriter<CycleDataType> &take_from) :
  _cycler(cycler),
  _pointer(take_from._pointer)
{
  take_from._pointer = (CycleDataType *)NULL;
  nassertv(_pointer != (CycleDataType *)NULL);
}

////////////////////////////////////////////////////////////////////
//     Function: CycleDataWriter::Constructor
//       Access: Public
//  Description: This flavor of the constructor elevates the pointer
//               from the CycleDataReader from a read to a write
//               pointer (and invalidates the reader).
////////////////////////////////////////////////////////////////////
template<class CycleDataType>
INLINE CycleDataWriter<CycleDataType>::
CycleDataWriter(PipelineCycler<CycleDataType> &cycler,
                CycleDataReader<CycleDataType> &take_from) :
  _cycler(cycler)
{
  _pointer = _cycler.elevate_read(take_from.take_pointer());
}

////////////////////////////////////////////////////////////////////
//     Function: CycleDataWriter::Destructor
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
template<class CycleDataType>
INLINE CycleDataWriter<CycleDataType>::
~CycleDataWriter() {
  if (_pointer != (CycleDataType *)NULL) {
    _cycler.release_write(_pointer);
  }
}

////////////////////////////////////////////////////////////////////
//     Function: CycleDataWriter::operator ->
//       Access: Public
//  Description: This provides an indirect member access to the actual
//               CycleData data.
////////////////////////////////////////////////////////////////////
template<class CycleDataType>
INLINE CycleDataType *CycleDataWriter<CycleDataType>::
operator -> () {
  nassertr(_pointer != (CycleDataType *)NULL, _cycler.cheat());
  return _pointer;
}

////////////////////////////////////////////////////////////////////
//     Function: CycleDataWriter::operator ->
//       Access: Public
//  Description: This provides an indirect member access to the actual
//               CycleData data.
////////////////////////////////////////////////////////////////////
template<class CycleDataType>
INLINE const CycleDataType *CycleDataWriter<CycleDataType>::
operator -> () const {
  nassertr(_pointer != (CycleDataType *)NULL, _cycler.cheat());
  return _pointer;
}

////////////////////////////////////////////////////////////////////
//     Function: CycleDataWriter::Typecast pointer
//       Access: Public
//  Description: This allows the CycleDataWriter to be passed to any
//               function that expects a CycleDataType pointer.
////////////////////////////////////////////////////////////////////
template<class CycleDataType>
INLINE CycleDataWriter<CycleDataType>::
operator CycleDataType * () {
  nassertr(_pointer != (CycleDataType *)NULL, _cycler.cheat());
  return _pointer;
}
