// Filename: recorderHeader.I
// Created by:  drose (29Jan04)
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


////////////////////////////////////////////////////////////////////
//     Function: RecorderHeader::Constructor
//       Access: Published
//  Description:
////////////////////////////////////////////////////////////////////
INLINE RecorderHeader::
RecorderHeader() {
  time(&_start_time);
  _random_seed = (int)_start_time;
}

////////////////////////////////////////////////////////////////////
//     Function: RecorderHeader::Copy Constructor
//       Access: Published
//  Description:
////////////////////////////////////////////////////////////////////
INLINE RecorderHeader::
RecorderHeader(const RecorderHeader &copy) :
  _start_time(copy._start_time),
  _random_seed(copy._random_seed)
{
}

////////////////////////////////////////////////////////////////////
//     Function: RecorderHeader::Copy Assignment Operator
//       Access: Published
//  Description:
////////////////////////////////////////////////////////////////////
INLINE void RecorderHeader::
operator = (const RecorderHeader &copy) {
  _start_time = copy._start_time;
  _random_seed = copy._random_seed;
}

////////////////////////////////////////////////////////////////////
//     Function: RecorderHeader::Destructor
//       Access: Published
//  Description:
////////////////////////////////////////////////////////////////////
INLINE RecorderHeader::
~RecorderHeader() {
}
