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
