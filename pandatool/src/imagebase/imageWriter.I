// Filename: imageWriter.I
// Created by:  drose (19Jun00)
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
//     Function: ImageWriter::write_image
//       Access: Public
//  Description: Writes the generated to the user's specified output
//               filename.
////////////////////////////////////////////////////////////////////
INLINE void ImageWriter::
write_image() {
  write_image(_image);
}
