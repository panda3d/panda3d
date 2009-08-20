// Filename: p3dTemporaryFile.I
// Created by:  drose (19Aug09)
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
//     Function: P3DTemporaryFile::get_filename
//       Access: Public
//  Description: Returns the temporary filename.
////////////////////////////////////////////////////////////////////
inline const string &P3DTemporaryFile::
get_filename() const {
  return _filename;
}
