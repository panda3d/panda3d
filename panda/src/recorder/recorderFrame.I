// Filename: recorderFrame.I
// Created by:  drose (28Jan04)
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
//     Function: RecorderFrame::Constructor
//       Access: Published
//  Description:
////////////////////////////////////////////////////////////////////
INLINE RecorderFrame::
RecorderFrame() {
}

////////////////////////////////////////////////////////////////////
//     Function: RecorderFrame::Constructor
//       Access: Published
//  Description:
////////////////////////////////////////////////////////////////////
INLINE RecorderFrame::
RecorderFrame(double timestamp, int frame,
              bool table_changed, RecorderTable *table) :
  _timestamp(timestamp),
  _frame(frame),
  _table_changed(table_changed),
  _table(table)
{
}

////////////////////////////////////////////////////////////////////
//     Function: RecorderFrame::Destructor
//       Access: Published
//  Description:
////////////////////////////////////////////////////////////////////
INLINE RecorderFrame::
~RecorderFrame() {
}
