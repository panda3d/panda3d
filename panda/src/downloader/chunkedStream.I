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
