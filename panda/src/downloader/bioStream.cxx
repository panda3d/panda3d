// Filename: bioStream.cxx
// Created by:  drose (25Sep02)
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

#include "bioStream.h"


#ifdef HAVE_OPENSSL

////////////////////////////////////////////////////////////////////
//     Function: IBioStream::is_closed
//       Access: Public, Virtual
//  Description: Returns true if the last eof condition was triggered
//               because the socket has genuinely closed, or false if
//               we can expect more data to come along shortly.
////////////////////////////////////////////////////////////////////
bool IBioStream::
is_closed() {
  if (!_buf._read_open) {
    return true;
  }
  clear();
  return false;
}

////////////////////////////////////////////////////////////////////
//     Function: IBioStream::close
//       Access: Public, Virtual
//  Description: Resets the BioStream to empty, but does not actually
//               close the source BIO unless owns_source was true.
////////////////////////////////////////////////////////////////////
void IBioStream::
close() {
  _buf.close();
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
  if (!_buf._write_open) {
    return true;
  }
  clear();
  return false;
}

////////////////////////////////////////////////////////////////////
//     Function: OBioStream::close
//       Access: Public, Virtual
//  Description: Resets the BioStream to empty, but does not actually
//               close the source BIO unless owns_source was true.
////////////////////////////////////////////////////////////////////
void OBioStream::
close() {
  _buf.close();
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
  if (!_buf._read_open) {
    return true;
  }
  clear();
  return false;
}

////////////////////////////////////////////////////////////////////
//     Function: BioStream::close
//       Access: Public, Virtual
//  Description: Resets the BioStream to empty, but does not actually
//               close the source BIO unless owns_source was true.
////////////////////////////////////////////////////////////////////
void BioStream::
close() {
  _buf.close();
}

////////////////////////////////////////////////////////////////////
//     Function: IBioStream::get_read_state
//       Access: Public, Virtual
//  Description: Returns an enum indicating how we are coming along in
//               reading the document.
////////////////////////////////////////////////////////////////////
IBioStream::ReadState IBioStream::
get_read_state() {
  // For an IBioStream, this method is meaningless, and always returns
  // RS_error.

  // This method is intended for those specialized streams that scan
  // through an HTTP document.
  return RS_error;
}

#endif  // HAVE_OPENSSL
