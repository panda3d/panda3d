// Filename: chunkedStreamBuf.I
// Created by:  drose (14Nov06)
//
////////////////////////////////////////////////////////////////////
//
// PANDA 3D SOFTWARE
// Copyright (c) 2001 - 2004, Disney Enterprises, Inc.  All rights reserved
//
// All use of this software is subject to the terms of the Panda 3d
// Software license.  You should have received a copy of this license
// along with this source code; you will also find a current copy of
// the license at http://etc.cmu.edu/panda3d/docs/license/ .
//
// To contact the maintainers of this program write to
// panda3d-general@lists.sourceforge.net .
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
