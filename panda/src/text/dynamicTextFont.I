// Filename: dynamicTextFont.I
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
//     Function: DynamicTextFont::get_name
//       Access: Published
//  Description: Disambiguates the get_name() method between that
//               inherited from TextFont and that inherited from
//               FreetypeFont.
////////////////////////////////////////////////////////////////////
INLINE const string &DynamicTextFont::
get_name() const {
  return TextFont::get_name();
}

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

  return FreetypeFont::set_point_size(point_size);
}

////////////////////////////////////////////////////////////////////
//     Function: DynamicTextFont::get_point_size
//       Access: Published
//  Description: Returns the point size of the font.
////////////////////////////////////////////////////////////////////
INLINE float DynamicTextFont::
get_point_size() const {
  return FreetypeFont::get_point_size();
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

  return FreetypeFont::set_pixels_per_unit(pixels_per_unit);
}

////////////////////////////////////////////////////////////////////
//     Function: DynamicTextFont::get_pixels_per_unit
//       Access: Published
//  Description: Returns the resolution of the texture map.  See
//               set_pixels_per_unit().
////////////////////////////////////////////////////////////////////
INLINE float DynamicTextFont::
get_pixels_per_unit() const {
  return FreetypeFont::get_pixels_per_unit();
}

////////////////////////////////////////////////////////////////////
//     Function: DynamicTextFont::set_scale_factor
//       Access: Published
//  Description: Sets the factor by which the font is rendered larger
//               by the FreeType library before being filtered down to
//               its actual size in the texture as specified by
//               set_pixels_per_unit().  This may be set to a number
//               larger than 1.0 to improve the font's antialiasing
//               (since FreeType doesn't really do a swell job of
//               antialiasing by itself).  There is some performance
//               implication for setting this different than 1.0.
//
//               This should only be called before any characters have
//               been requested out of the font, or immediately after
//               calling clear().
////////////////////////////////////////////////////////////////////
INLINE bool DynamicTextFont::
set_scale_factor(float scale_factor) {
  // If this assertion fails, you didn't call clear() first.  RTFM.
  nassertr(get_num_pages() == 0, false);

  return FreetypeFont::set_scale_factor(scale_factor);
}

////////////////////////////////////////////////////////////////////
//     Function: DynamicTextFont::get_scale_factor
//       Access: Published
//  Description: Returns the antialiasing scale factor.  See
//               set_scale_factor().
////////////////////////////////////////////////////////////////////
INLINE float DynamicTextFont::
get_scale_factor() const {
  return FreetypeFont::get_scale_factor();
}

////////////////////////////////////////////////////////////////////
//     Function: DynamicTextFont::set_native_antialias
//       Access: Published
//  Description: Sets whether the Freetype library's built-in
//               antialias mode is enabled.  There are two unrelated
//               ways to achieve antialiasing: with Freetype's native
//               antialias mode, and with the use of a scale_factor
//               greater than one.  By default, both modes are
//               enabled.
//
//               At low resolutions, some fonts may do better with one
//               mode or the other.  In general, Freetype's native
//               antialiasing will produce less blurry results, but
//               may introduce more artifacts.
////////////////////////////////////////////////////////////////////
INLINE void DynamicTextFont::
set_native_antialias(bool native_antialias) {
  // If this assertion fails, you didn't call clear() first.  RTFM.
  nassertv(get_num_pages() == 0);

  FreetypeFont::set_native_antialias(native_antialias);
}

////////////////////////////////////////////////////////////////////
//     Function: DynamicTextFont::get_native_antialias
//       Access: Published
//  Description: Returns whether Freetype's built-in antialias mode is
//               enabled.  See set_native_antialias().
////////////////////////////////////////////////////////////////////
INLINE bool DynamicTextFont::
get_native_antialias() const {
  return FreetypeFont::get_native_antialias();
}

////////////////////////////////////////////////////////////////////
//     Function: DynamicTextFont::get_font_pixel_size
//       Access: Published
//  Description: This is used to report whether the requested pixel
//               size is being only approximated by a fixed-pixel-size
//               font.  This returns 0 in the normal case, in which a
//               scalable font is used, or the fixed-pixel-size font
//               has exactly the requested pixel size.
//
//               If this returns non-zero, it is the pixel size of the
//               font that we are using to approximate our desired
//               size.
////////////////////////////////////////////////////////////////////
INLINE int DynamicTextFont::
get_font_pixel_size() const {
  return FreetypeFont::get_font_pixel_size();
}

////////////////////////////////////////////////////////////////////
//     Function: DynamicTextFont::get_line_height
//       Access: Published
//  Description: Returns the number of units high each line of text
//               is.
////////////////////////////////////////////////////////////////////
INLINE float DynamicTextFont::
get_line_height() const {
  return TextFont::get_line_height();
}

////////////////////////////////////////////////////////////////////
//     Function: DynamicTextFont::get_space_advance
//       Access: Published
//  Description: Returns the number of units wide a space is.
////////////////////////////////////////////////////////////////////
INLINE float DynamicTextFont::
get_space_advance() const {
  return TextFont::get_space_advance();
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
//               (but it should be set somewhat smaller than this
//               number, which is controlled by set_texture_margin(),
//               to prevent bleed-in from neighboring letters in the
//               texture).
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

////////////////////////////////////////////////////////////////////
//     Function: DynamicTextFont::set_minfilter
//       Access: Published
//  Description: Sets the filter type used when minimizing the
//               textures created for this font.
////////////////////////////////////////////////////////////////////
INLINE void DynamicTextFont::
set_minfilter(Texture::FilterType filter) {
  _minfilter = filter;
  update_filters();
}

////////////////////////////////////////////////////////////////////
//     Function: DynamicTextFont::get_minfilter
//       Access: Published
//  Description: Returns the filter type used when minimizing the
//               textures created for this font.
////////////////////////////////////////////////////////////////////
INLINE Texture::FilterType DynamicTextFont::
get_minfilter() const {
  return _minfilter;
}

////////////////////////////////////////////////////////////////////
//     Function: DynamicTextFont::set_magfilter
//       Access: Published
//  Description: Sets the filter type used when enlarging the
//               textures created for this font.
////////////////////////////////////////////////////////////////////
INLINE void DynamicTextFont::
set_magfilter(Texture::FilterType filter) {
  _magfilter = filter;
  update_filters();
}

////////////////////////////////////////////////////////////////////
//     Function: DynamicTextFont::get_magfilter
//       Access: Published
//  Description: Returns the filter type used when enlarging the
//               textures created for this font.
////////////////////////////////////////////////////////////////////
INLINE Texture::FilterType DynamicTextFont::
get_magfilter() const {
  return _magfilter;
}

////////////////////////////////////////////////////////////////////
//     Function: DynamicTextFont::set_anisotropic_degree
//       Access: Published
//  Description: Enables or disables anisotropic filtering on the
//               textures created for this font.  The default value is
//               usually 1, or off.  See
//               Texture::set_anisotropic_degree().
////////////////////////////////////////////////////////////////////
INLINE void DynamicTextFont::
set_anisotropic_degree(int anisotropic_degree) {
  _anisotropic_degree = anisotropic_degree;
  update_filters();
}

////////////////////////////////////////////////////////////////////
//     Function: DynamicTextFont::get_anisotropic_degree
//       Access: Published
//  Description: Returns the current anisotropic degree for textures
//               created for this font.  See set_anisotropic_degree().
////////////////////////////////////////////////////////////////////
INLINE int DynamicTextFont::
get_anisotropic_degree() const {
  return _anisotropic_degree;
}

////////////////////////////////////////////////////////////////////
//     Function: DynamicTextFont::set_update_cleared_glyphs
//       Access: Published, Static
//  Description: Sets the flag indicating whether texture memory
//               should be updated immediately as old glyphs are
//               removed.  If this is true, texture memory will be
//               immediately updated when old glyphs are removed from
//               the pages.  If this is false (the default), texture
//               memory may not be updated until the page is next
//               written to, that is, the next time a glyph is
//               recorded on that page.
//
//               Most of the time, there is no reason to set this
//               true, unless you are debugging the DynamicTextFont
//               code and want to be able to see exactly what is in
//               each texture map at any given time.
//
//               This is a global flag across all DynamicTextFont
//               objects.
////////////////////////////////////////////////////////////////////
INLINE void DynamicTextFont::
set_update_cleared_glyphs(bool update_cleared_glyphs) {
  _update_cleared_glyphs = update_cleared_glyphs;
}

////////////////////////////////////////////////////////////////////
//     Function: DynamicTextFont::get_update_cleared_glyphs
//       Access: Published, Static
//  Description: Returns the flag indicating whether texture memory
//               should be updated immediately as old glyphs are
//               removed.  See set_update_cleared_glyphs().
////////////////////////////////////////////////////////////////////
INLINE bool DynamicTextFont::
get_update_cleared_glyphs() {
  return _update_cleared_glyphs;
}

INLINE ostream &
operator << (ostream &out, const DynamicTextFont &dtf) {
  return out << dtf.get_name();
}
