// Filename: p3dPackage.I
// Created by:  drose (12Jun09)
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
//     Function: P3DPackage::get_ready
//       Access: Public
//  Description: Returns true if the package has been downloaded and
//               verified and is ready to be used, false if it has
//               not.
////////////////////////////////////////////////////////////////////
inline bool P3DPackage::
get_ready() const {
  return _ready;
}

////////////////////////////////////////////////////////////////////
//     Function: P3DPackage::get_failed
//       Access: Public
//  Description: Returns true if the package cannot be made ready, for
//               instance because the download server is down.
////////////////////////////////////////////////////////////////////
inline bool P3DPackage::
get_failed() const {
  return _failed;
}

////////////////////////////////////////////////////////////////////
//     Function: P3DPackage::get_package_dir
//       Access: Public
//  Description: Returns the directory into which this package is
//               installed.
////////////////////////////////////////////////////////////////////
inline const string &P3DPackage::
get_package_dir() const {
  return _package_dir;
}

////////////////////////////////////////////////////////////////////
//     Function: P3DPackage::decode_hexdigit
//       Access: Private
//  Description: Returns the integer value corresponding to the
//               indicated hex digit.  Returns -1 if it is not a hex
//               digit.
////////////////////////////////////////////////////////////////////
inline int P3DPackage::
decode_hexdigit(char c) {
  if (isdigit(c)) {
    return c - '0';
  }
  c = tolower(c);
  if (c >= 'a' && c <= 'f') {
    return c - 'a' + 10;
  }

  return -1;
}

////////////////////////////////////////////////////////////////////
//     Function: P3DPackage::encode_hexdigit
//       Access: Private
//  Description: Returns the hex digit corresponding to the
//               indicated integer value.
////////////////////////////////////////////////////////////////////
inline char P3DPackage::
encode_hexdigit(int c) {
  if (c >= 10) {
    return c - 10 + 'a';
  }
  return c + '0';
}
