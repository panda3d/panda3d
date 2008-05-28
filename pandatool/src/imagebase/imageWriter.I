// Filename: imageWriter.I
// Created by:  drose (19Jun00)
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
//     Function: ImageWriter::write_image
//       Access: Public
//  Description: Writes the generated to the user's specified output
//               filename.
////////////////////////////////////////////////////////////////////
INLINE void ImageWriter::
write_image() {
  write_image(_image);
}
