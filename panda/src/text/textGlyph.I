// Filename: textGlyph.I
// Created by:  drose (08Feb02)
//
////////////////////////////////////////////////////////////////////
//
// PANDA 3D SOFTWARE
// Copyright (c) 2001, Disney Enterprises, Inc.  All rights reserved
//
// All use of this software is subject to the terms of the Panda 3d
// Software license.  You should have received a copy of this license
// along with this source code; you will also find a current copy of
// the license at http://www.panda3d.org/license.txt .
//
// To contact the maintainers of this program write to
// panda3d@yahoogroups.com .
//
////////////////////////////////////////////////////////////////////


////////////////////////////////////////////////////////////////////
//     Function: TextGlyph::Default constructor
//       Access: Public
//  Description: This constructor makes an invalid glyph.
////////////////////////////////////////////////////////////////////
INLINE TextGlyph::
TextGlyph() {
  _geom = (Geom *)NULL;
  _advance = 0.0f;
}

////////////////////////////////////////////////////////////////////
//     Function: TextGlyph::Constructor
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
INLINE TextGlyph::
TextGlyph(Geom *geom, const AllTransitionsWrapper &trans, float advance) :
  _geom(geom), _trans(trans), _advance(advance) 
{ 
}

////////////////////////////////////////////////////////////////////
//     Function: TextGlyph::Copy Constructor
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
INLINE TextGlyph::
TextGlyph(const TextGlyph &copy) :
  _geom(copy._geom),
  _trans(copy._trans),
  _advance(copy._advance) 
{ 
}

////////////////////////////////////////////////////////////////////
//     Function: TextGlyph::Copy Assignment Operator
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
INLINE void TextGlyph::
operator = (const TextGlyph &copy) {
  _geom = copy._geom;
  _trans = copy._trans;
  _advance = copy._advance;
}

////////////////////////////////////////////////////////////////////
//     Function: TextGlyph::get_geom
//       Access: Public
//  Description: Returns a Geom that renders the particular glyph.
////////////////////////////////////////////////////////////////////
INLINE Geom *TextGlyph::
get_geom() const {
  return _geom;
}

////////////////////////////////////////////////////////////////////
//     Function: TextGlyph::get_trans
//       Access: Public
//  Description: Returns the state transitions that should be applied
//               to the glyph geometry to render it correctly.
////////////////////////////////////////////////////////////////////
INLINE const AllTransitionsWrapper &TextGlyph::
get_trans() const {
  return _trans;
}

////////////////////////////////////////////////////////////////////
//     Function: TextGlyph::get_advance
//       Access: Public
//  Description: Returns the distance by which the character pointer
//               should be advanced after placing this character;
//               i.e. the approximate width the character takes up on
//               the line.
////////////////////////////////////////////////////////////////////
INLINE float TextGlyph::
get_advance() const {
  return _advance;
}
