// Filename: chunkedStreamBuf.I
// Created by:  drose (14Nov06)
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
//     Function: ChunkedStreamBuf::is_closed
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
INLINE bool ChunkedStreamBuf::
is_closed() const {
  return (_source == (BioStreamPtr *)NULL || (*_source)->is_closed());
}

////////////////////////////////////////////////////////////////////
//     Function: ChunkedStreamBuf::get_read_state
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
INLINE ISocketStream::ReadState ChunkedStreamBuf::
get_read_state() const {
  return _read_state;
}
