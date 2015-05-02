// Filename: handleStreamBuf.I
// Created by:  drose (05Jun09)
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
//     Function: HandleStreamBuf::get_handle
//       Access: Public
//  Description: Returns the handle that was passed to open_read() or
//               open_write().
////////////////////////////////////////////////////////////////////
inline FHandle HandleStreamBuf::
get_handle() const {
  return _handle;
}

////////////////////////////////////////////////////////////////////
//     Function: HandleStreamBuf::has_gdata
//       Access: Public
//  Description: Returns true if there is data in the stream's "get"
//               buffer, meaning that at least one character can be
//               extracted from the stream without making an OS read()
//               call.  Returns false if the get buffer is empty,
//               meaning the next read call will hit the OS.
////////////////////////////////////////////////////////////////////
inline bool HandleStreamBuf::
has_gdata() const {
  return (egptr() != gptr());
}
