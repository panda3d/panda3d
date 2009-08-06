// Filename: encryptStream.I
// Created by:  drose (01Sep04)
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
//     Function: IDecryptStream::Constructor
//       Access: Published
//  Description:
////////////////////////////////////////////////////////////////////
INLINE IDecryptStream::
IDecryptStream() : istream(&_buf) {
}

////////////////////////////////////////////////////////////////////
//     Function: IDecryptStream::Constructor
//       Access: Published
//  Description:
////////////////////////////////////////////////////////////////////
INLINE IDecryptStream::
IDecryptStream(istream *source, bool owns_source,
               const string &password) : istream(&_buf) {
  open(source, owns_source, password);
}

////////////////////////////////////////////////////////////////////
//     Function: IDecryptStream::open
//       Access: Published
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
//       Access: Published
//  Description: Resets the EncryptStream to empty, but does not actually
//               close the source istream unless owns_source was true.
////////////////////////////////////////////////////////////////////
INLINE IDecryptStream &IDecryptStream::
close() {
  _buf.close_read();
  return *this;
}

////////////////////////////////////////////////////////////////////
//     Function: IDecryptStream::get_algorithm
//       Access: Published
//  Description: Returns the encryption algorithm that was read from
//               the stream.
////////////////////////////////////////////////////////////////////
INLINE const string &IDecryptStream::
get_algorithm() const {
  return _buf.get_algorithm();
}

////////////////////////////////////////////////////////////////////
//     Function: IDecryptStream::get_key_length
//       Access: Published
//  Description: Returns the encryption key length, in bits, that was
//               read from the stream.
////////////////////////////////////////////////////////////////////
INLINE int IDecryptStream::
get_key_length() const {
  return _buf.get_key_length();
}

////////////////////////////////////////////////////////////////////
//     Function: IDecryptStream::get_iteration_count
//       Access: Published
//  Description: Returns the value that was was read from the stream.
////////////////////////////////////////////////////////////////////
INLINE int IDecryptStream::
get_iteration_count() const {
  return _buf.get_iteration_count();
}


////////////////////////////////////////////////////////////////////
//     Function: OEncryptStream::Constructor
//       Access: Published
//  Description:
////////////////////////////////////////////////////////////////////
INLINE OEncryptStream::
OEncryptStream() : ostream(&_buf) {
}

////////////////////////////////////////////////////////////////////
//     Function: OEncryptStream::Constructor
//       Access: Published
//  Description:
////////////////////////////////////////////////////////////////////
INLINE OEncryptStream::
OEncryptStream(ostream *dest, bool owns_dest, const string &password) :
  ostream(&_buf) 
{
  open(dest, owns_dest, password);
}

////////////////////////////////////////////////////////////////////
//     Function: OEncryptStream::open
//       Access: Published
//  Description: 
////////////////////////////////////////////////////////////////////
INLINE OEncryptStream &OEncryptStream::
open(ostream *dest, bool owns_dest, const string &password) {
  clear((ios_iostate)0);
  _buf.open_write(dest, owns_dest, password);
  return *this;
}

////////////////////////////////////////////////////////////////////
//     Function: OEncryptStream::close
//       Access: Published
//  Description: Resets the EncryptStream to empty, but does not actually
//               close the dest ostream unless owns_dest was true.
////////////////////////////////////////////////////////////////////
INLINE OEncryptStream &OEncryptStream::
close() {
  _buf.close_write();
  return *this;
}


////////////////////////////////////////////////////////////////////
//     Function: OEncryptStream::set_algorithm
//       Access: Published
//  Description: Specifies the encryption algorithm that should be
//               used for future calls to open().  The default
//               is whatever is specified by the encryption-algorithm
//               config variable.  The complete set of available
//               algorithms is defined by the current version of
//               OpenSSL.
//
//               If an invalid algorithm is specified, there is no
//               immediate error return code, but open() will
//               fail.
////////////////////////////////////////////////////////////////////
INLINE void OEncryptStream::
set_algorithm(const string &algorithm) {
  _buf.set_algorithm(algorithm);
}

////////////////////////////////////////////////////////////////////
//     Function: OEncryptStream::set_key_length
//       Access: Published
//  Description: Specifies the length of the key, in bits, that should
//               be used to encrypt the stream in future calls to
//               open().  The default is whatever is specified
//               by the encryption-key-length config variable.  
//
//               If an invalid key_length for the chosen algorithm is
//               specified, there is no immediate error return code,
//               but open() will fail.
////////////////////////////////////////////////////////////////////
INLINE void OEncryptStream::
set_key_length(int key_length) {
  _buf.set_key_length(key_length);
}

////////////////////////////////////////////////////////////////////
//     Function: OEncryptStream::set_iteration_count
//       Access: Published
//  Description: Specifies the number of times to repeatedly hash the
//               key before writing it to the stream in future calls
//               to open().  Its purpose is to make it
//               computationally more expensive for an attacker to
//               search the key space exhaustively.  This should be a
//               multiple of 1,000 and should not exceed about 65
//               million; the value 0 indicates just one application
//               of the hashing algorithm.
//
//               The default is whatever is specified by the
//               encryption-iteration-count config variable.
////////////////////////////////////////////////////////////////////
INLINE void OEncryptStream::
set_iteration_count(int iteration_count) {
  _buf.set_iteration_count(iteration_count);
}
