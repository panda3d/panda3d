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
  _pointer = (CycleDataType *)_cycler.write();
}

////////////////////////////////////////////////////////////////////
//     Function: CycleDataWriter::Copy Constructor
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
template<class CycleDataType>
INLINE CycleDataWriter<CycleDataType>::
CycleDataWriter(const CycleDataWriter<CycleDataType> &copy) :
  _cycler(copy._cycler),
  _pointer(copy._pointer)
{
  _cycler.increment_write(_pointer);
}

////////////////////////////////////////////////////////////////////
//     Function: CycleDataWriter::Destructor
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
template<class CycleDataType>
INLINE CycleDataWriter<CycleDataType>::
~CycleDataWriter() {
  _cycler.release_write(_pointer);
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
  return _pointer;
}
