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
IIdentityStream() : ISocketStream(&_buf) {
}

////////////////////////////////////////////////////////////////////
//     Function: IIdentityStream::Constructor
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
INLINE IIdentityStream::
IIdentityStream(BioStreamPtr *source, HTTPChannel *doc,
                bool has_content_length, size_t content_length) : 
  ISocketStream(&_buf) 
{
  open(source, doc, has_content_length, content_length);
}

////////////////////////////////////////////////////////////////////
//     Function: IIdentityStream::open
//       Access: Public
//  Description: 
////////////////////////////////////////////////////////////////////
INLINE IIdentityStream &IIdentityStream::
open(BioStreamPtr *source, HTTPChannel *doc, 
     bool has_content_length, size_t content_length) {
  clear((ios_iostate)0);
  _buf.open_read(source, doc, has_content_length, content_length);
  return *this;
}
