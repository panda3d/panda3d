// Filename: textAssembler.I
// Created by:  drose (06Apr04)
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
//     Function: TextAssembler::get_ul
//       Access: Public
//  Description: Returns the upper-left corner of the assembled text,
//               in 2-d text coordinates.
////////////////////////////////////////////////////////////////////
INLINE const LVector2f &TextAssembler::
get_ul() const {
  return _ul;
}

////////////////////////////////////////////////////////////////////
//     Function: TextAssembler::get_lr
//       Access: Public
//  Description: Returns the lower-right corner of the assembled text,
//               in 2-d text coordinates.
////////////////////////////////////////////////////////////////////
INLINE const LVector2f &TextAssembler::
get_lr() const {
  return _lr;
}

////////////////////////////////////////////////////////////////////
//     Function: TextAssembler::get_num_rows
//       Access: Public
//  Description: Returns the number of rows of text after it has all
//               been wordwrapped and assembled.
////////////////////////////////////////////////////////////////////
INLINE int TextAssembler::
get_num_rows() const {
  return _num_rows;
}

////////////////////////////////////////////////////////////////////
//     Function: TextAssembler::calc_width
//       Access: Private, Static
//  Description: Returns the width of a single character, according to
//               its associated font.
////////////////////////////////////////////////////////////////////
INLINE float TextAssembler::
calc_width(const TextCharacter &tch) {
  return calc_width(tch._character, *tch._properties);
}

////////////////////////////////////////////////////////////////////
//     Function: TextAssembler::TextCharacter::Constructor
//       Access: Public
//  Description: 
////////////////////////////////////////////////////////////////////
INLINE TextAssembler::TextCharacter::
TextCharacter(wchar_t character, const TextProperties *properties) :
  _character(character),
  _properties(properties)
{
}

////////////////////////////////////////////////////////////////////
//     Function: TextAssembler::GlyphPlacement::add_piece
//       Access: Public
//  Description: Adds a piece of the glyph, consisting of a single
//               Geom and an associated RenderState.  Typically, a
//               glyph will have exactly one piece; there will only be
//               multiple pieces in the case of cheesy accent marks or
//               ligatures.
////////////////////////////////////////////////////////////////////
INLINE void TextAssembler::GlyphPlacement::
add_piece(Geom *geom, const RenderState *state) {
  Piece piece;
  piece._geom = geom;
  piece._state = state;
  _pieces.push_back(piece);
}
