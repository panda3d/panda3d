// Filename: cycleDataReader.I
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
//     Function: CycleDataReader::Constructor
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
template<class CycleDataType>
INLINE CycleDataReader<CycleDataType>::
CycleDataReader(const PipelineCycler<CycleDataType> &cycler) :
  _cycler(cycler)
{
  _pointer = _cycler.read();
}

////////////////////////////////////////////////////////////////////
//     Function: CycleDataReader::Copy Constructor
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
template<class CycleDataType>
INLINE CycleDataReader<CycleDataType>::
CycleDataReader(const CycleDataReader<CycleDataType> &copy) :
  _cycler(copy._cycler),
  _pointer(copy._pointer)
{
  _cycler.increment_read(_pointer);
}

////////////////////////////////////////////////////////////////////
//     Function: CycleDataReader::Destructor
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
template<class CycleDataType>
INLINE CycleDataReader<CycleDataType>::
~CycleDataReader() {
  _cycler.release_read(_pointer);
}

////////////////////////////////////////////////////////////////////
//     Function: CycleDataReader::operator ->
//       Access: Public
//  Description: This provides an indirect member access to the actual
//               CycleData data.
////////////////////////////////////////////////////////////////////
template<class CycleDataType>
INLINE const CycleDataType *CycleDataReader<CycleDataType>::
operator -> () const {
  return _pointer;
}
