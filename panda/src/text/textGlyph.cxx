// Filename: textGlyph.cxx
// Created by:  drose (08Feb02)
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

#include "textGlyph.h"

TypeHandle TextGlyph::_type_handle;

////////////////////////////////////////////////////////////////////
//     Function: TextGlyph::Destructor
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
TextGlyph::
~TextGlyph() {
}

////////////////////////////////////////////////////////////////////
//     Function: TextGlyph::is_whitespace
//       Access: Public, Virtual
//  Description: Returns true if this glyph represents invisible
//               whitespace, or false if it corresponds to some
//               visible character.
////////////////////////////////////////////////////////////////////
bool TextGlyph::
is_whitespace() const {
  // In a static font, there is no explicit glyph for whitespace, so
  // all glyphs are non-whitespace.
  return false;
}
