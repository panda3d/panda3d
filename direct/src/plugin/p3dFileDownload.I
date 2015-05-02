// Filename: p3dFileDownload.I
// Created by:  drose (11Jun09)
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
//     Function: P3DFileDownload::get_filename
//       Access: Public
//  Description: Returns the filename that we are downloading into.
////////////////////////////////////////////////////////////////////
const string &P3DFileDownload::
get_filename() const {
  return _filename;
}
