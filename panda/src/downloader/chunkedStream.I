// Filename: chunkedStream.I
// Created by:  drose (25Sep02)
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
//     Function: IChunkedStream::Constructor
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
INLINE IChunkedStream::
IChunkedStream() : ISocketStream(&_buf) {
}

////////////////////////////////////////////////////////////////////
//     Function: IChunkedStream::Constructor
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
INLINE IChunkedStream::
IChunkedStream(BioStreamPtr *source, HTTPChannel *doc) : ISocketStream(&_buf) {
  open(source, doc);
}

////////////////////////////////////////////////////////////////////
//     Function: IChunkedStream::open
//       Access: Public
//  Description: 
////////////////////////////////////////////////////////////////////
INLINE IChunkedStream &IChunkedStream::
open(BioStreamPtr *source, HTTPChannel *doc) {
  clear((ios_iostate)0);
  _buf.open_read(source, doc);
  return *this;
}
