// Filename: identityStream.cxx
// Created by:  drose (09Oct02)
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

#include "identityStream.h"

// This module is not compiled if OpenSSL is not available.
#ifdef HAVE_SSL

////////////////////////////////////////////////////////////////////
//     Function: IIdentityStream::is_closed
//       Access: Public, Virtual
//  Description: Returns true if the last eof condition was triggered
//               because the socket has genuinely closed, or false if
//               we can expect more data to come along shortly.
////////////////////////////////////////////////////////////////////
bool IIdentityStream::
is_closed() {
  if ((_buf._has_content_length && _buf._bytes_remaining == 0) || 
      (*_buf._source)->is_closed()) {
    return true;
  }
  clear();
  return false;
}

////////////////////////////////////////////////////////////////////
//     Function: IIdentityStream::close
//       Access: Public, Virtual
//  Description: Resets the IdentityStream to empty, but does not actually
//               close the source BIO unless owns_source was true.
////////////////////////////////////////////////////////////////////
void IIdentityStream::
close() {
  _buf.close_read();
}

#endif  // HAVE_SSL
