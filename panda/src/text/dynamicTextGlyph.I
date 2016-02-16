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

////////////////////////////////////////////////////////////////////
//     Function: DynamicTextGlyph::Constructor
//       Access: Public
//  Description: 
////////////////////////////////////////////////////////////////////
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

////////////////////////////////////////////////////////////////////
//     Function: DynamicTextGlyph::Constructor
//       Access: Public
//  Description: This constructor makes an empty glyph, whose only
//               purpose is to remember its width.  It has no bitmap
//               and no Geom.
////////////////////////////////////////////////////////////////////
INLINE DynamicTextGlyph::
DynamicTextGlyph(int character, PN_stdfloat advance) :
  TextGlyph(character, advance),
  _page((DynamicTextPage *)NULL),
  _x(0), _y(0),
  _x_size(0), _y_size(0),
  _margin(0)
{
}

////////////////////////////////////////////////////////////////////
//     Function: DynamicTextGlyph::Copy Constructor
//       Access: Private
//  Description: Copying DynamicTextGlyph objects is not allowed.
////////////////////////////////////////////////////////////////////
INLINE DynamicTextGlyph::
DynamicTextGlyph(const DynamicTextGlyph &) :
  TextGlyph(0)
{
  nassertv(false);
}

////////////////////////////////////////////////////////////////////
//     Function: DynamicTextGlyph::Copy Assignment Operator
//       Access: Private
//  Description: Copying DynamicTextGlyph objects is not allowed.
////////////////////////////////////////////////////////////////////
INLINE void DynamicTextGlyph::
operator = (const DynamicTextGlyph &) {
  nassertv(false);
}


////////////////////////////////////////////////////////////////////
//     Function: DynamicTextGlyph::get_page
//       Access: Published
//  Description: Returns the DynamicTextPage that this glyph is on.
////////////////////////////////////////////////////////////////////
INLINE DynamicTextPage *DynamicTextGlyph::
get_page() const {
  return _page;
}

////////////////////////////////////////////////////////////////////
//     Function: DynamicTextGlyph::intersects
//       Access: Published
//  Description: Returns true if the particular position this glyph
//               has been assigned to overlaps the rectangle whose
//               top left corner is at x, y and whose size is given by
//               x_size, y_size, or false otherwise.
////////////////////////////////////////////////////////////////////
INLINE bool DynamicTextGlyph::
intersects(int x, int y, int x_size, int y_size) const {
  int hright = x + x_size;
  int hbot = y + y_size;

  int mright = _x + _x_size;
  int mbot = _y + _y_size;

  return !(x >= mright || hright <= _x ||
           y >= mbot || hbot <= _y);
}

////////////////////////////////////////////////////////////////////
//     Function: DynamicTextGlyph::get_left
//       Access: Published
//  Description: Returns the vertex coordinates that can be used when
//               creating a custom text renderer.
////////////////////////////////////////////////////////////////////
INLINE PN_stdfloat DynamicTextGlyph::
get_left() const {
  return _quad_dimensions[0];
}

////////////////////////////////////////////////////////////////////
//     Function: DynamicTextGlyph::get_bottom
//       Access: Published
//  Description: Returns the vertex coordinates that can be used when
//               creating a custom text renderer.
////////////////////////////////////////////////////////////////////
INLINE PN_stdfloat DynamicTextGlyph::
get_bottom() const {
  return _quad_dimensions[1];
}

////////////////////////////////////////////////////////////////////
//     Function: DynamicTextGlyph::get_right
//       Access: Published
//  Description: Returns the vertex coordinates that can be used when
//               creating a custom text renderer.
////////////////////////////////////////////////////////////////////
INLINE PN_stdfloat DynamicTextGlyph::
get_right() const {
  return _quad_dimensions[2];
}

////////////////////////////////////////////////////////////////////
//     Function: DynamicTextGlyph::get_top
//       Access: Published
//  Description: Returns the vertex coordinates that can be used when
//               creating a custom text renderer.
////////////////////////////////////////////////////////////////////
INLINE PN_stdfloat DynamicTextGlyph::
get_top() const {
  return _quad_dimensions[3];
}

////////////////////////////////////////////////////////////////////
//     Function: DynamicTextGlyph::get_uv_left
//       Access: Published
//  Description: Returns the UV coordinates that can be used when
//               creating a custom text renderer.
////////////////////////////////////////////////////////////////////
INLINE PN_stdfloat DynamicTextGlyph::
get_uv_left() const {
  return _quad_texcoords[0];
}

////////////////////////////////////////////////////////////////////
//     Function: DynamicTextGlyph::get_uv_bottom
//       Access: Published
//  Description: Returns the UV coordinates that can be used when
//               creating a custom text renderer.
////////////////////////////////////////////////////////////////////
INLINE PN_stdfloat DynamicTextGlyph::
get_uv_bottom() const {
  return _quad_texcoords[1];
}

////////////////////////////////////////////////////////////////////
//     Function: DynamicTextGlyph::get_uv_right
//       Access: Published
//  Description: Returns the UV coordinates that can be used when
//               creating a custom text renderer.
////////////////////////////////////////////////////////////////////
INLINE PN_stdfloat DynamicTextGlyph::
get_uv_right() const {
  return _quad_texcoords[2];
}

////////////////////////////////////////////////////////////////////
//     Function: DynamicTextGlyph::get_uv_top
//       Access: Published
//  Description: Returns the UV coordinates that can be used when
//               creating a custom text renderer.
////////////////////////////////////////////////////////////////////
INLINE PN_stdfloat DynamicTextGlyph::
get_uv_top() const {
  return _quad_texcoords[3];
}
