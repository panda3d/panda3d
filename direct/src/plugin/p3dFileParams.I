// Filename: p3dFileParams.I
// Created by:  drose (23Jun09)
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
//     Function: P3DFileParams::get_p3d_filename
//       Access: Public
//  Description: Returns the p3d filename that was passed to the
//               constructor.
////////////////////////////////////////////////////////////////////
inline const string &P3DFileParams::
get_p3d_filename() const {
  return _p3d_filename;
}
