// Filename: bioStream.cxx
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

#include "bioStream.h"


#ifdef HAVE_SSL

////////////////////////////////////////////////////////////////////
//     Function: IBioStream::is_closed
//       Access: Public, Virtual
//  Description: Returns true if the last eof condition was triggered
//               because the socket has genuinely closed, or false if
//               we can expect more data to come along shortly.
////////////////////////////////////////////////////////////////////
bool IBioStream::
is_closed() {
  if (_buf._is_closed) {
    return true;
  }
  clear();
  return false;
}

////////////////////////////////////////////////////////////////////
//     Function: OBioStream::is_closed
//       Access: Public, Virtual
//  Description: Returns true if the last write fail condition was
//               triggered because the socket has genuinely closed, or
//               false if we can expect to send more data along
//               shortly.
////////////////////////////////////////////////////////////////////
bool OBioStream::
is_closed() {
  if (_buf._is_closed) {
    return true;
  }
  clear();
  return false;
}

////////////////////////////////////////////////////////////////////
//     Function: BioStream::is_closed
//       Access: Public, Virtual
//  Description: Returns true if the last eof or failure condition was
//               triggered because the socket has genuinely closed, or
//               false if we can expect to read or send more data
//               shortly.
////////////////////////////////////////////////////////////////////
bool BioStream::
is_closed() {
  if (_buf._is_closed) {
    return true;
  }
  clear();
  return false;
}

#endif  // HAVE_SSL
