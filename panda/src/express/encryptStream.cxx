// Filename: encryptStream.cxx
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

#include "encryptStream.h"

////////////////////////////////////////////////////////////////////
//     Function: encrypt_string
//       Access: Published
//  Description: Encrypts the indicated source string using the given
//               password and algorithm (or empty string for the
//               default algorithm).  Returns the encrypted string.
////////////////////////////////////////////////////////////////////
string
encrypt_string(const string &source, const string &password,
               const string &algorithm) {
  ostringstream output;

  {
    OEncryptStream encrypt(&output, false, password, algorithm);
    encrypt.write(source.data(), source.length());
  }

  return output.str();
}

////////////////////////////////////////////////////////////////////
//     Function: decrypt_string
//       Access: Published
//  Description: Decrypts the previously-encrypted string using the
//               given password (which must be the same password
//               passed to encrypt()).  The return value is the
//               decrypted string.  Note that a decryption error,
//               including an incorrect password, cannot easily be
//               detected, and the return value may simply be a
//               garbage string.
////////////////////////////////////////////////////////////////////
string
decrypt_string(const string &source, const string &password) {
  istringstream input(source);
  ostringstream output;

  {
    IDecryptStream decrypt(&input, false, password);
    
    int ch = decrypt.get();
    while (!decrypt.eof() && !decrypt.fail()) {
      output.put(ch);
      ch = decrypt.get();
    }
  }

  return output.str();
}
