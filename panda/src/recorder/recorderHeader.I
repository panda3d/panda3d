// Filename: recorderHeader.I
// Created by:  drose (29Jan04)
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
