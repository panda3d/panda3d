// Filename: textGlyph.I
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
//     Function: TextGlyph::Default constructor
//       Access: Public
//  Description: This constructor makes an invalid glyph.
////////////////////////////////////////////////////////////////////
INLINE TextGlyph::
TextGlyph(int character) :
  _character(character)
{
  _geom = (Geom *)NULL;
  _advance = 0.0f;
}

////////////////////////////////////////////////////////////////////
//     Function: TextGlyph::Constructor
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
INLINE TextGlyph::
TextGlyph(int character, const Geom *geom, 
          const RenderState *state, PN_stdfloat advance) :
  _character(character),
  _geom(geom), 
  _state(state),
  _advance(advance) 
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
  _character(copy._character),
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
  _character = copy._character;
  _geom = copy._geom;
  _state = copy._state;
  _advance = copy._advance;
}

////////////////////////////////////////////////////////////////////
//     Function: TextGlyph::get_character
//       Access: Public
//  Description: Returns the Unicode value that corresponds to the
//               character this glyph represents.
////////////////////////////////////////////////////////////////////
INLINE int TextGlyph::
get_character() const {
  return _character;
}

////////////////////////////////////////////////////////////////////
//     Function: TextGlyph::get_geom
//       Access: Public
//  Description: Returns a Geom that renders the particular glyph.
////////////////////////////////////////////////////////////////////
INLINE PT(Geom) TextGlyph::
get_geom(Geom::UsageHint usage_hint) const {
  if (_geom == (Geom *)NULL) {
    return NULL;
  }

  // We always return a copy of the geom.  That will allow the caller
  // to modify its vertices without fear of stomping on other copies;
  // it is also critical for the DynamicTextGlyph, which depends on
  // this behavior to properly count references to this glyph.
  PT(Geom) new_geom = _geom->make_copy();
  new_geom->set_usage_hint(usage_hint);
  const GeomVertexData *vdata = new_geom->get_vertex_data();
  nassertr(vdata != NULL, new_geom);
  if (vdata->get_usage_hint() != usage_hint) {
    new_geom->modify_vertex_data()->set_usage_hint(usage_hint);
  }
  return new_geom;
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
INLINE PN_stdfloat TextGlyph::
get_advance() const {
  return _advance;
}
