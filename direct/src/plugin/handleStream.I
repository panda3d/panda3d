// Filename: handleStream.I
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
//     Function: HandleStream::Constructor
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
inline HandleStream::
HandleStream() : iostream(&_buf) {
}

////////////////////////////////////////////////////////////////////
//     Function: HandleStream::Destructor
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
inline HandleStream::
~HandleStream() {
  close();
}

////////////////////////////////////////////////////////////////////
//     Function: HandleStream::open_read
//       Access: Public
//  Description: Attempts to open the given handle for input.  The
//               stream may not be simultaneously open for input and
//               output.
////////////////////////////////////////////////////////////////////
inline void HandleStream::
open_read(FHandle handle) {
  clear((ios::iostate)0);
  _buf.open_read(handle);
  if (!_buf.is_open_read()) {
    clear(ios::failbit);
  }
}

////////////////////////////////////////////////////////////////////
//     Function: HandleStream::open_write
//       Access: Public
//  Description: Attempts to open the given handle for output.  The
//               stream may not be simultaneously open for input and
//               output.
////////////////////////////////////////////////////////////////////
inline void HandleStream::
open_write(FHandle handle) {
  clear((ios::iostate)0);
  _buf.open_write(handle);
  if (!_buf.is_open_write()) {
    clear(ios::failbit);
  }
}

////////////////////////////////////////////////////////////////////
//     Function: HandleStream::close
//       Access: Public
//  Description: 
////////////////////////////////////////////////////////////////////
inline void HandleStream::
close() {
  _buf.close();
}

////////////////////////////////////////////////////////////////////
//     Function: HandleStream::get_handle
//       Access: Public
//  Description: Returns the handle that was passed to open_read() or
//               open_write().
////////////////////////////////////////////////////////////////////
inline FHandle HandleStream::
get_handle() const {
  return _buf.get_handle();
}
