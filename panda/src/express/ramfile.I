// Filename: ramfile.I
// Created by:  mike (09Jan97)
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
//     Function: Ramfile::constructor
//       Access: Published
//  Description:
////////////////////////////////////////////////////////////////////
INLINE Ramfile::
Ramfile() {
  _pos = 0;
}

////////////////////////////////////////////////////////////////////
//     Function: Ramfile::seek
//       Access: Published
//  Description: Moves the data pointer to the indicated byte
//               position.  It is not an error to move the pointer
//               past the end of data.
////////////////////////////////////////////////////////////////////
INLINE void Ramfile::
seek(size_t pos) {
  _pos = pos;
}

////////////////////////////////////////////////////////////////////
//     Function: Ramfile::tell
//       Access: Published
//  Description: Returns the current data pointer position as a byte
//               offset from the beginning of the stream.
////////////////////////////////////////////////////////////////////
INLINE size_t Ramfile::
tell() const {
  return _pos;
}

////////////////////////////////////////////////////////////////////
//     Function: Ramfile::get_data
//       Access: Published
//  Description: Returns the entire buffer contents as a string,
//               regardless of the current data pointer.
////////////////////////////////////////////////////////////////////
INLINE const string &Ramfile::
get_data() const {
  return _data;
}
