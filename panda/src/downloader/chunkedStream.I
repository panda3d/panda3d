// Filename: chunkedStream.I
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
//     Function: IChunkedStream::Constructor
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
INLINE IChunkedStream::
IChunkedStream() : istream(&_buf) {
}

////////////////////////////////////////////////////////////////////
//     Function: IChunkedStream::Constructor
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
INLINE IChunkedStream::
IChunkedStream(istream *source, bool owns_source) : istream(&_buf) {
  open(source, owns_source);
}

////////////////////////////////////////////////////////////////////
//     Function: IChunkedStream::open
//       Access: Public
//  Description: 
////////////////////////////////////////////////////////////////////
INLINE IChunkedStream &IChunkedStream::
open(istream *source, bool owns_source) {
  clear(0);
  _buf.open_read(source, owns_source);
  return *this;
}

////////////////////////////////////////////////////////////////////
//     Function: IChunkedStream::close
//       Access: Public
//  Description: Resets the ChunkedStream to empty, but does not actually
//               close the source CHUNKED unless owns_source was true.
////////////////////////////////////////////////////////////////////
INLINE IChunkedStream &IChunkedStream::
close() {
  _buf.close_read();
  return *this;
}
