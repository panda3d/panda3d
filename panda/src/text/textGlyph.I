// Filename: textGlyph.I
// Created by:  drose (08Feb02)
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
TextGlyph(const Geom *geom, const RenderState *state, float advance) :
  _geom(geom), _state(state), _advance(advance) 
{ 
  if (_state == (RenderState *)NULL) {
    _state = RenderState::make_empty();
  }
}

////////////////////////////////////////////////////////////////////
//     Function: TextGlyph::Copy Constructor
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
INLINE TextGlyph::
TextGlyph(const TextGlyph &copy) :
  _geom(copy._geom),
  _state(copy._state),
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
  _state = copy._state;
  _advance = copy._advance;
}

////////////////////////////////////////////////////////////////////
//     Function: TextGlyph::get_geom
//       Access: Public
//  Description: Returns a Geom that renders the particular glyph.
////////////////////////////////////////////////////////////////////
INLINE PT(Geom) TextGlyph::
get_geom() const {
  if (_geom == (Geom *)NULL) {
    return NULL;
  }

  // We always return a copy of the geom.  That will allow the caller
  // to modify its vertices without fear of stomping on other copies;
  // it is also critical for the DynamicTextGlyph, which depends on
  // this behavior to properly count references to this glyph.
  return _geom->make_copy();
}

////////////////////////////////////////////////////////////////////
//     Function: TextGlyph::get_state
//       Access: Public
//  Description: Returns the state in which the glyph should be
//               rendered.
////////////////////////////////////////////////////////////////////
INLINE const RenderState *TextGlyph::
get_state() const {
  return _state;
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
