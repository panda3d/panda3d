// Filename: encrypt_string.cxx
// Created by:  drose (30Jan07)
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

#include "encrypt_string.h"
#include "encryptStream.h"

#ifdef HAVE_OPENSSL
////////////////////////////////////////////////////////////////////
//     Function: encrypt_string
//       Access: Published
//  Description: Encrypts the indicated source string using the given
//               password.  Returns the encrypted string.
////////////////////////////////////////////////////////////////////
string
encrypt_string(const string &source, const string &password) {
  ostringstream output;

  {
    OEncryptStream encrypt(&output, false, password);
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

#endif // HAVE_OPENSSL
