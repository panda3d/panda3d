// Filename: encryptStream.I
// Created by:  drose (01Sep04)
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
//     Function: IDecryptStream::Constructor
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
INLINE IDecryptStream::
IDecryptStream() : istream(&_buf) {
}

////////////////////////////////////////////////////////////////////
//     Function: IDecryptStream::Constructor
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
INLINE IDecryptStream::
IDecryptStream(istream *source, bool owns_source,
               const string &password) : istream(&_buf) {
  open(source, owns_source, password);
}

////////////////////////////////////////////////////////////////////
//     Function: IDecryptStream::open
//       Access: Public
//  Description: 
////////////////////////////////////////////////////////////////////
INLINE IDecryptStream &IDecryptStream::
open(istream *source, bool owns_source, const string &password) {
  clear((ios_iostate)0);
  _buf.open_read(source, owns_source, password);
  return *this;
}

////////////////////////////////////////////////////////////////////
//     Function: IDecryptStream::close
//       Access: Public
//  Description: Resets the EncryptStream to empty, but does not actually
//               close the source istream unless owns_source was true.
////////////////////////////////////////////////////////////////////
INLINE IDecryptStream &IDecryptStream::
close() {
  _buf.close_read();
  return *this;
}


////////////////////////////////////////////////////////////////////
//     Function: OEncryptStream::Constructor
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
INLINE OEncryptStream::
OEncryptStream() : ostream(&_buf) {
}

////////////////////////////////////////////////////////////////////
//     Function: OEncryptStream::Constructor
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
INLINE OEncryptStream::
OEncryptStream(ostream *dest, bool owns_dest, const string &password,
               const string &encryption_algorithm) :
  ostream(&_buf) 
{
  open(dest, owns_dest, password, encryption_algorithm);
}

////////////////////////////////////////////////////////////////////
//     Function: OEncryptStream::open
//       Access: Public
//  Description: 
////////////////////////////////////////////////////////////////////
INLINE OEncryptStream &OEncryptStream::
open(ostream *dest, bool owns_dest, const string &password,
     const string &encryption_algorithm) {
  clear((ios_iostate)0);
  _buf.open_write(dest, owns_dest, password, encryption_algorithm);
  return *this;
}

////////////////////////////////////////////////////////////////////
//     Function: OEncryptStream::close
//       Access: Public
//  Description: Resets the EncryptStream to empty, but does not actually
//               close the dest ostream unless owns_dest was true.
////////////////////////////////////////////////////////////////////
INLINE OEncryptStream &OEncryptStream::
close() {
  _buf.close_write();
  return *this;
}

