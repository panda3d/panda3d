// Filename: fileSpec.I
// Created by:  drose (29Jun09)
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
//     Function: FileSpec::get_filename
//       Access: Private
//  Description: Returns the relative path to this file on disk,
//               within the package root directory.
////////////////////////////////////////////////////////////////////
inline const string &FileSpec::
get_filename() const {
  return _filename;
}

////////////////////////////////////////////////////////////////////
//     Function: FileSpec::get_pathname
//       Access: Private
//  Description: Returns the full path to this file on disk.
////////////////////////////////////////////////////////////////////
inline string FileSpec::
get_pathname(const string &package_dir) const {
  return package_dir + "/" + _filename;
}

////////////////////////////////////////////////////////////////////
//     Function: FileSpec::get_size
//       Access: Private
//  Description: Returns the expected size of this file on disk, in
//               bytes.
////////////////////////////////////////////////////////////////////
inline size_t FileSpec::
get_size() const {
  return _size;
}

////////////////////////////////////////////////////////////////////
//     Function: FileSpec::decode_hexdigit
//       Access: Private
//  Description: Returns the integer value corresponding to the
//               indicated hex digit.  Returns -1 if it is not a hex
//               digit.
////////////////////////////////////////////////////////////////////
inline int FileSpec::
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
//     Function: FileSpec::encode_hexdigit
//       Access: Private
//  Description: Returns the hex digit corresponding to the
//               indicated integer value.
////////////////////////////////////////////////////////////////////
inline char FileSpec::
encode_hexdigit(int c) {
  if (c >= 10) {
    return c - 10 + 'a';
  }
  return c + '0';
}
