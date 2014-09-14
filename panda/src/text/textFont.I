// Filename: textFont.I
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


////////////////////////////////////////////////////////////////////
//     Function: TextFont::is_valid
//       Access: Published
//  Description: Returns true if the font is valid and ready to use,
//               false otherwise.
////////////////////////////////////////////////////////////////////
INLINE bool TextFont::
is_valid() const {
  return _is_valid;
}

////////////////////////////////////////////////////////////////////
//     Function: TextFont::operator bool
//       Access: Published
//  Description: Returns true if the font is valid and ready to use,
//               false otherwise.
////////////////////////////////////////////////////////////////////
INLINE TextFont::
operator bool () const {
  return is_valid();
}

////////////////////////////////////////////////////////////////////
//     Function: TextFont::get_line_height
//       Access: Published
//  Description: Returns the number of units high each line of text
//               is.
////////////////////////////////////////////////////////////////////
INLINE PN_stdfloat TextFont::
get_line_height() const {
  return _line_height;
}

////////////////////////////////////////////////////////////////////
//     Function: TextFont::set_line_height
//       Access: Published
//  Description: Changes the number of units high each line of text
//               is.
////////////////////////////////////////////////////////////////////
INLINE void TextFont::
set_line_height(PN_stdfloat line_height) {
  _line_height = line_height;
}

////////////////////////////////////////////////////////////////////
//     Function: TextFont::get_space_advance
//       Access: Published
//  Description: Returns the number of units wide a space is.
////////////////////////////////////////////////////////////////////
INLINE PN_stdfloat TextFont::
get_space_advance() const {
  return _space_advance;
}

////////////////////////////////////////////////////////////////////
//     Function: TextFont::set_space_advance
//       Access: Published
//  Description: Changes the number of units wide a space is.
////////////////////////////////////////////////////////////////////
INLINE void TextFont::
set_space_advance(PN_stdfloat space_advance) {
  _space_advance = space_advance;
}

////////////////////////////////////////////////////////////////////
//     Function: DynamicTextFont::get_glyph
//       Access: Public, Virtual
//  Description: Gets the glyph associated with the given character
//               code, as well as an optional scaling parameter that
//               should be applied to the glyph's geometry and advance
//               parameters.  Returns the glyph on success.  On failure,
//               it may still return a printable glyph, or it may
//               return NULL.
////////////////////////////////////////////////////////////////////
INLINE const TextGlyph *TextFont::
get_glyph(int character) {
  const TextGlyph *glyph = NULL;
  get_glyph(character, glyph);
  return glyph;
}
