// Filename: bioStream.I
// Created by:  drose (25Sep02)
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
//     Function: IBioStream::Constructor
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
INLINE IBioStream::
IBioStream() : istream(&_buf) {
}

////////////////////////////////////////////////////////////////////
//     Function: IBioStream::Constructor
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
INLINE IBioStream::
IBioStream(BIO *source, bool owns_source) : istream(&_buf) {
  open(source, owns_source);
}

////////////////////////////////////////////////////////////////////
//     Function: IBioStream::open
//       Access: Public
//  Description: 
////////////////////////////////////////////////////////////////////
INLINE IBioStream &IBioStream::
open(BIO *source, bool owns_source) {
  clear(0);
  _buf.open_read(source, owns_source);
  return *this;
}

////////////////////////////////////////////////////////////////////
//     Function: IBioStream::close
//       Access: Public
//  Description: Resets the BioStream to empty, but does not actually
//               close the source BIO unless owns_source was true.
////////////////////////////////////////////////////////////////////
INLINE IBioStream &IBioStream::
close() {
  _buf.close_read();
  return *this;
}
