// Filename: identityStream.I
// Created by:  drose (09Oct02)
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
//     Function: IIdentityStream::Constructor
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
INLINE IIdentityStream::
IIdentityStream() : istream(&_buf) {
}

////////////////////////////////////////////////////////////////////
//     Function: IIdentityStream::Constructor
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
INLINE IIdentityStream::
IIdentityStream(istream *source, bool owns_source, 
               HTTPDocument *doc, size_t content_length) : istream(&_buf) {
  open(source, owns_source, doc, content_length);
}

////////////////////////////////////////////////////////////////////
//     Function: IIdentityStream::open
//       Access: Public
//  Description: 
////////////////////////////////////////////////////////////////////
INLINE IIdentityStream &IIdentityStream::
open(istream *source, bool owns_source, HTTPDocument *doc, 
     size_t content_length) {
  clear(0);
  _buf.open_read(source, owns_source, doc, content_length);
  return *this;
}

////////////////////////////////////////////////////////////////////
//     Function: IIdentityStream::close
//       Access: Public
//  Description: Resets the IdentityStream to empty, but does not actually
//               close the source IDENTITY unless owns_source was true.
////////////////////////////////////////////////////////////////////
INLINE IIdentityStream &IIdentityStream::
close() {
  _buf.close_read();
  return *this;
}
