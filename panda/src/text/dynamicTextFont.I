// Filename: dynamicTextFont.I
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
//     Function: DynamicTextFont::set_point_size
//       Access: Published
//  Description: Sets the point size of the font.  This controls the
//               apparent size of the font onscreen.  By convention, a
//               10 point font is about 1 screen unit high.
//
//               This should only be called before any characters have
//               been requested out of the font, or immediately after
//               calling clear().
////////////////////////////////////////////////////////////////////
INLINE bool DynamicTextFont::
set_point_size(float point_size) {
  // If this assertion fails, you didn't call clear() first.  RTFM.
  nassertr(get_num_pages() == 0, false);

  _point_size = point_size;
  return reset_scale();
}

////////////////////////////////////////////////////////////////////
//     Function: DynamicTextFont::get_point_size
//       Access: Published
//  Description: Returns the point size of the font.
////////////////////////////////////////////////////////////////////
INLINE float DynamicTextFont::
get_point_size() const {
  return _point_size;
}

////////////////////////////////////////////////////////////////////
//     Function: DynamicTextFont::set_pixels_per_unit
//       Access: Published
//  Description: Set the resolution of the texture map, and hence the
//               clarity of the resulting font.  This sets the number
//               of pixels in the texture map that are used for each
//               onscreen unit.
//
//               Setting this number larger results in an easier to
//               read font, but at the cost of more texture memory.
//
//               This should only be called before any characters have
//               been requested out of the font, or immediately after
//               calling clear().
////////////////////////////////////////////////////////////////////
INLINE bool DynamicTextFont::
set_pixels_per_unit(float pixels_per_unit) {
  // If this assertion fails, you didn't call clear() first.  RTFM.
  nassertr(get_num_pages() == 0, false);

  _pixels_per_unit = pixels_per_unit;
  return reset_scale();
}

////////////////////////////////////////////////////////////////////
//     Function: DynamicTextFont::get_pixels_per_unit
//       Access: Published
//  Description: Returns the resolution of the texture map.  See
//               set_pixels_per_unit().
////////////////////////////////////////////////////////////////////
INLINE float DynamicTextFont::
get_pixels_per_unit() const {
  return _pixels_per_unit;
}

////////////////////////////////////////////////////////////////////
//     Function: DynamicTextFont::set_texture_margin
//       Access: Published
//  Description: Sets the number of pixels of padding that is added
//               around the border of each glyph before adding it to
//               the texture map.  This reduces the bleed in from
//               neighboring glyphs in the texture map.
////////////////////////////////////////////////////////////////////
INLINE void DynamicTextFont::
set_texture_margin(int texture_margin) {
  _texture_margin = texture_margin;
}

////////////////////////////////////////////////////////////////////
//     Function: DynamicTextFont::get_texture_margin
//       Access: Published
//  Description: Returns the number of pixels of padding that is added
//               around the border of each glyph in the texture map.
//               See set_texture_margin().
////////////////////////////////////////////////////////////////////
INLINE int DynamicTextFont::
get_texture_margin() const {
  return _texture_margin;
}

////////////////////////////////////////////////////////////////////
//     Function: DynamicTextFont::set_poly_margin
//       Access: Published
//  Description: Sets the number of pixels of padding that is included
//               around each glyph in the generated polygons.  This
//               helps prevent the edges of the glyphs from being cut
//               off at small minifications.  It is not related to the
//               amount of extra pixels reserved in the texture map
//               (but it should be no larger than this number, which
//               is controlled by set_texture_margin()).
////////////////////////////////////////////////////////////////////
INLINE void DynamicTextFont::
set_poly_margin(float poly_margin) {
  _poly_margin = poly_margin;
}

////////////////////////////////////////////////////////////////////
//     Function: DynamicTextFont::get_poly_margin
//       Access: Published
//  Description: Returns the number of pixels of padding that is
//               included around each glyph in the generated polygons.
//               See set_poly_margin().
////////////////////////////////////////////////////////////////////
INLINE float DynamicTextFont::
get_poly_margin() const {
  return _poly_margin;
}

////////////////////////////////////////////////////////////////////
//     Function: DynamicTextFont::set_page_size
//       Access: Published
//  Description: Sets the x, y size of the textures that are created
//               for the DynamicTextFont.
////////////////////////////////////////////////////////////////////
INLINE void DynamicTextFont::
set_page_size(int x_size, int y_size) {
  _page_x_size = x_size;
  _page_y_size = y_size;
}

////////////////////////////////////////////////////////////////////
//     Function: DynamicTextFont::get_page_x_size
//       Access: Published
//  Description: Returns the x size of the textures that are created
//               for the DynamicTextFont.  See set_page_size().
////////////////////////////////////////////////////////////////////
INLINE int DynamicTextFont::
get_page_x_size() const {
  return _page_x_size;
}

////////////////////////////////////////////////////////////////////
//     Function: DynamicTextFont::get_page_y_size
//       Access: Published
//  Description: Returns the y size of the textures that are created
//               for the DynamicTextFont.  See set_page_size().
////////////////////////////////////////////////////////////////////
INLINE int DynamicTextFont::
get_page_y_size() const {
  return _page_y_size;
}
