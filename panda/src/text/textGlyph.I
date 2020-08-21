/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file textGlyph.I
 * @author drose
 * @date 2002-02-08
 */

/**
 * This constructor makes an empty glyph.
 */
INLINE TextGlyph::
TextGlyph(int character, PN_stdfloat advance) :
  _character(character),
  _geom(nullptr),
  _advance(advance),
  _has_quad(false)
{
}

/**
 *
 */
INLINE TextGlyph::
TextGlyph(int character, const Geom *geom,
          const RenderState *state, PN_stdfloat advance) :
  _character(character),
  _geom(geom),
  _state(state),
  _advance(advance),
  _has_quad(false)
{
  if (geom != nullptr) {
    check_quad_geom();
  }
  if (_state == nullptr) {
    _state = RenderState::make_empty();
  }
}

/**
 *
 */
INLINE TextGlyph::
TextGlyph(const TextGlyph &copy) :
  _character(copy._character),
  _geom(copy._geom),
  _state(copy._state),
  _advance(copy._advance),
  _has_quad(copy._has_quad),
  _quad_dimensions(copy._quad_dimensions),
  _quad_texcoords(copy._quad_texcoords)
{
}

/**
 *
 */
INLINE void TextGlyph::
operator = (const TextGlyph &copy) {
  _character = copy._character;
  _geom = copy._geom;
  _state = copy._state;
  _advance = copy._advance;
  _has_quad = copy._has_quad;
  _quad_dimensions = copy._quad_dimensions;
  _quad_texcoords = copy._quad_texcoords;
}

/**
 * Returns the Unicode value that corresponds to the character this glyph
 * represents.
 */
INLINE int TextGlyph::
get_character() const {
  return _character;
}

/**
 * Returns true if this glyph contains the definition for a simple quad,
 * rather than a more complex piece of geometry.
 *
 * You may still call get_geom() even if this returns true, which will
 * synthesize a Geom for this quad.
 */
INLINE bool TextGlyph::
has_quad() const {
  return _has_quad;
}

/**
 * Assuming that this glyph is representable as a textured quad, returns its
 * dimensions and UV range.  Returns false if it is not representable as a
 * quad, or if it is whitespace.
 *
 * The order of the components is left, bottom, right, top.
 */
INLINE bool TextGlyph::
get_quad(LVecBase4 &dimensions, LVecBase4 &texcoords) const {
  if (!_has_quad) {
    return false;
  }

  dimensions = _quad_dimensions;
  texcoords = _quad_texcoords;
  return true;
}

/**
 * Returns the state in which the glyph should be rendered.
 */
INLINE const RenderState *TextGlyph::
get_state() const {
  return _state;
}

/**
 * Returns the distance by which the character pointer should be advanced
 * after placing this character; i.e.  the approximate width the character
 * takes up on the line.
 */
INLINE PN_stdfloat TextGlyph::
get_advance() const {
  return _advance;
}
