// Filename: freetypeFont.I
// Created by:  drose (07Sep03)
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
//     Function: FreetypeFont::Destructor
//       Access: Public
//  Description: 
////////////////////////////////////////////////////////////////////
INLINE FreetypeFont::
~FreetypeFont() {
  unload_font();
}

////////////////////////////////////////////////////////////////////
//     Function: FreetypeFont::set_point_size
//       Access: Public
//  Description: Sets the point size of the font.  This controls the
//               apparent size of the font onscreen.  By convention, a
//               10 point font is about 1 screen unit high.
//
//               This should only be called before any characters have
//               been requested out of the font, or immediately after
//               calling clear().
////////////////////////////////////////////////////////////////////
INLINE bool FreetypeFont::
set_point_size(float point_size) {
  _point_size = point_size;
  return reset_scale();
}

////////////////////////////////////////////////////////////////////
//     Function: FreetypeFont::get_point_size
//       Access: Public
//  Description: Returns the point size of the font.
////////////////////////////////////////////////////////////////////
INLINE float FreetypeFont::
get_point_size() const {
  return _point_size;
}

////////////////////////////////////////////////////////////////////
//     Function: FreetypeFont::set_pixels_per_unit
//       Access: Public
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
INLINE bool FreetypeFont::
set_pixels_per_unit(float pixels_per_unit) {
  _tex_pixels_per_unit = pixels_per_unit;
  return reset_scale();
}

////////////////////////////////////////////////////////////////////
//     Function: FreetypeFont::get_pixels_per_unit
//       Access: Public
//  Description: Returns the resolution of the texture map.  See
//               set_pixels_per_unit().
////////////////////////////////////////////////////////////////////
INLINE float FreetypeFont::
get_pixels_per_unit() const {
  return _tex_pixels_per_unit;
}

////////////////////////////////////////////////////////////////////
//     Function: FreetypeFont::set_pixel_size
//       Access: Public
//  Description: Computes the appropriate pixels_per_unit value to set
//               the size of the font in the texture to the indicated
//               number of pixels.  This is just another way to
//               specify pixels_per_unit().
////////////////////////////////////////////////////////////////////
INLINE bool FreetypeFont::
set_pixel_size(float pixel_size) {
  return set_pixels_per_unit(pixel_size * _points_per_unit / _point_size);
}

////////////////////////////////////////////////////////////////////
//     Function: FreetypeFont::get_pixel_size
//       Access: Public
//  Description: Returns the size of the font in pixels, as it appears
//               in the texture.
////////////////////////////////////////////////////////////////////
INLINE float FreetypeFont::
get_pixel_size() const {
  return _tex_pixels_per_unit * _point_size / _points_per_unit;
}

////////////////////////////////////////////////////////////////////
//     Function: FreetypeFont::set_scale_factor
//       Access: Public
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
INLINE bool FreetypeFont::
set_scale_factor(float scale_factor) {
  _scale_factor = scale_factor;
  return reset_scale();
}

////////////////////////////////////////////////////////////////////
//     Function: FreetypeFont::get_scale_factor
//       Access: Public
//  Description: Returns the antialiasing scale factor.  See
//               set_scale_factor().
////////////////////////////////////////////////////////////////////
INLINE float FreetypeFont::
get_scale_factor() const {
  return _scale_factor;
}

////////////////////////////////////////////////////////////////////
//     Function: FreetypeFont::set_native_antialias
//       Access: Public
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
INLINE void FreetypeFont::
set_native_antialias(bool native_antialias) {
  _native_antialias = native_antialias;
}

////////////////////////////////////////////////////////////////////
//     Function: FreetypeFont::get_native_antialias
//       Access: Public
//  Description: Returns whether Freetype's built-in antialias mode is
//               enabled.  See set_native_antialias().
////////////////////////////////////////////////////////////////////
INLINE bool FreetypeFont::
get_native_antialias() const {
  return _native_antialias;
}

////////////////////////////////////////////////////////////////////
//     Function: FreetypeFont::get_line_height
//       Access: Public
//  Description: Returns the number of units high each line of text
//               is.
////////////////////////////////////////////////////////////////////
INLINE float FreetypeFont::
get_line_height() const {
  return _line_height;
}

////////////////////////////////////////////////////////////////////
//     Function: FreetypeFont::get_space_advance
//       Access: Public
//  Description: Returns the number of units wide a space is.
////////////////////////////////////////////////////////////////////
INLINE float FreetypeFont::
get_space_advance() const {
  return _space_advance;
}

////////////////////////////////////////////////////////////////////
//     Function: FreetypeFont::get_points_per_unit
//       Access: Public, Static
//  Description: Returns the point size of the font that is one Panda
//               unit high.  This is an arbitrary Panda convention for
//               text, and is set to 10.0.
////////////////////////////////////////////////////////////////////
INLINE float FreetypeFont::
get_points_per_unit() {
  return _points_per_unit;
}

////////////////////////////////////////////////////////////////////
//     Function: FreetypeFont::get_points_per_inch
//       Access: Public, Static
//  Description: Returns the number of points in one inch.  This is a
//               universal typographic convention.
////////////////////////////////////////////////////////////////////
INLINE float FreetypeFont::
get_points_per_inch() {
  return _points_per_inch;
}
