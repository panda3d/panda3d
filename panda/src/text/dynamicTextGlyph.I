/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file dynamicTextGlyph.I
 * @author drose
 * @date 2002-02-09
 */

/**
 *
 */
INLINE DynamicTextGlyph::
DynamicTextGlyph(int character, DynamicTextPage *page, int x, int y,
                 int x_size, int y_size, int margin, PN_stdfloat advance) :
  TextGlyph(character, advance),
  _page(page),
  _x(x), _y(y),
  _x_size(x_size), _y_size(y_size),
  _margin(margin)
{
}

/**
 * This constructor makes an empty glyph, whose only purpose is to remember
 * its width.  It has no bitmap and no Geom.
 */
INLINE DynamicTextGlyph::
DynamicTextGlyph(int character, PN_stdfloat advance) :
  TextGlyph(character, advance),
  _page(nullptr),
  _x(0), _y(0),
  _x_size(0), _y_size(0),
  _margin(0)
{
}

/**
 * Returns the DynamicTextPage that this glyph is on.
 */
INLINE DynamicTextPage *DynamicTextGlyph::
get_page() const {
  return _page;
}

/**
 * Returns true if the particular position this glyph has been assigned to
 * overlaps the rectangle whose top left corner is at x, y and whose size is
 * given by x_size, y_size, or false otherwise.
 */
INLINE bool DynamicTextGlyph::
intersects(int x, int y, int x_size, int y_size) const {
  int hright = x + x_size;
  int hbot = y + y_size;

  int mright = _x + _x_size;
  int mbot = _y + _y_size;

  return !(x >= mright || hright <= _x ||
           y >= mbot || hbot <= _y);
}

/**
 * Returns the vertex coordinates that can be used when creating a custom text
 * renderer.
 */
INLINE PN_stdfloat DynamicTextGlyph::
get_left() const {
  return _quad_dimensions[0];
}

/**
 * Returns the vertex coordinates that can be used when creating a custom text
 * renderer.
 */
INLINE PN_stdfloat DynamicTextGlyph::
get_bottom() const {
  return _quad_dimensions[1];
}

/**
 * Returns the vertex coordinates that can be used when creating a custom text
 * renderer.
 */
INLINE PN_stdfloat DynamicTextGlyph::
get_right() const {
  return _quad_dimensions[2];
}

/**
 * Returns the vertex coordinates that can be used when creating a custom text
 * renderer.
 */
INLINE PN_stdfloat DynamicTextGlyph::
get_top() const {
  return _quad_dimensions[3];
}

/**
 * Returns the UV coordinates that can be used when creating a custom text
 * renderer.
 */
INLINE PN_stdfloat DynamicTextGlyph::
get_uv_left() const {
  return _quad_texcoords[0];
}

/**
 * Returns the UV coordinates that can be used when creating a custom text
 * renderer.
 */
INLINE PN_stdfloat DynamicTextGlyph::
get_uv_bottom() const {
  return _quad_texcoords[1];
}

/**
 * Returns the UV coordinates that can be used when creating a custom text
 * renderer.
 */
INLINE PN_stdfloat DynamicTextGlyph::
get_uv_right() const {
  return _quad_texcoords[2];
}

/**
 * Returns the UV coordinates that can be used when creating a custom text
 * renderer.
 */
INLINE PN_stdfloat DynamicTextGlyph::
get_uv_top() const {
  return _quad_texcoords[3];
}
