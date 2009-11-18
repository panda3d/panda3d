// Filename: freetypeFace.I
// Created by:  gogg (16Nov09)
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
//     Function: TextFont::get_face
//       Access: Published
//  Description: Retrieves the internal freetype face.
////////////////////////////////////////////////////////////////////
INLINE FT_Face FreetypeFace::
get_face() {
  return _face;
}

////////////////////////////////////////////////////////////////////
//     Function: TextFont::set_face
//       Access: Published
//  Description: Sets the internal freetype face.
////////////////////////////////////////////////////////////////////
INLINE void FreetypeFace::
set_face(FT_Face face) {
  _face = face;
}
