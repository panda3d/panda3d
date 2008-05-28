// Filename: recorderBase.cxx
// Created by:  drose (24Jan04)
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

#include "recorderBase.h"

TypeHandle RecorderBase::_type_handle;

////////////////////////////////////////////////////////////////////
//     Function: RecorderBase::Constructor
//       Access: Protected
//  Description:
////////////////////////////////////////////////////////////////////
RecorderBase::
RecorderBase() {
  _flags = 0;
}

////////////////////////////////////////////////////////////////////
//     Function: RecorderBase::Destructor
//       Access: Published, Virtual
//  Description:
////////////////////////////////////////////////////////////////////
RecorderBase::
~RecorderBase() {
  nassertv(_flags == 0);
}

////////////////////////////////////////////////////////////////////
//     Function: RecorderBase::record_frame
//       Access: Public, Virtual
//  Description: Records the most recent data collected into the
//               indicated datagram.
////////////////////////////////////////////////////////////////////
void RecorderBase::
record_frame(BamWriter *, Datagram &) {
}

////////////////////////////////////////////////////////////////////
//     Function: RecorderBase::play_frame
//       Access: Public, Virtual
//  Description: Reloads the most recent data collected from the
//               indicated datagram.
////////////////////////////////////////////////////////////////////
void RecorderBase::
play_frame(DatagramIterator &scan, BamReader *manager) {
}

////////////////////////////////////////////////////////////////////
//     Function: RecorderBase::write_recorder
//       Access: Public, Virtual
//  Description: Writes the contents of this object to the datagram
//               for encoding in the session file.  This is very
//               similar to write_datagram() for TypedWritable
//               objects, but it is used specifically to write the
//               Recorder object when generating the session file.  In
//               many cases, it will be the same as write_datagram().
//
//               This balances with fillin_recorder().
////////////////////////////////////////////////////////////////////
void RecorderBase::
write_recorder(BamWriter *, Datagram &) {
}

////////////////////////////////////////////////////////////////////
//     Function: RecorderBase::fillin_recorder
//       Access: Protected
//  Description: This internal function is called by make_recorder (in
//               derived classes) to read in all of the relevant data
//               from the session file.  It balances with
//               write_recorder().
////////////////////////////////////////////////////////////////////
void RecorderBase::
fillin_recorder(DatagramIterator &, BamReader *) {
}
