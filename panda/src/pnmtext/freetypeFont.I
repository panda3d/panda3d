/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file freetypeFont.I
 * @author drose
 * @date 2003-09-07
 */

/**
 *
 */
INLINE FreetypeFont::
~FreetypeFont() {
  unload_font();
}

/**
 * Sets the point size of the font.  This controls the apparent size of the
 * font onscreen.  By convention, a 10 point font is about 1 screen unit high.
 *
 * This should only be called before any characters have been requested out of
 * the font, or immediately after calling clear().
 */
INLINE bool FreetypeFont::
set_point_size(PN_stdfloat point_size) {
  _point_size = point_size;
  return reset_scale();
}

/**
 * Returns the point size of the font.
 */
INLINE PN_stdfloat FreetypeFont::
get_point_size() const {
  return _point_size;
}

/**
 * Set the resolution of the texture map, and hence the clarity of the
 * resulting font.  This sets the number of pixels in the texture map that are
 * used for each onscreen unit.
 *
 * Setting this number larger results in an easier to read font, but at the
 * cost of more texture memory.
 *
 * This should only be called before any characters have been requested out of
 * the font, or immediately after calling clear().
 */
INLINE bool FreetypeFont::
set_pixels_per_unit(PN_stdfloat pixels_per_unit) {
  _requested_pixels_per_unit = pixels_per_unit;
  return reset_scale();
}

/**
 * Returns the resolution of the texture map.  See set_pixels_per_unit().
 */
INLINE PN_stdfloat FreetypeFont::
get_pixels_per_unit() const {
  return _tex_pixels_per_unit;
}

/**
 * Computes the appropriate pixels_per_unit value to set the size of the font
 * in the texture to the indicated number of pixels.  This is just another way
 * to specify pixels_per_unit().
 */
INLINE bool FreetypeFont::
set_pixel_size(PN_stdfloat pixel_size) {
  return set_pixels_per_unit(pixel_size * _points_per_unit / _point_size);
}

/**
 * Returns the size of the font in pixels, as it appears in the texture.
 */
INLINE PN_stdfloat FreetypeFont::
get_pixel_size() const {
  return _tex_pixels_per_unit * _point_size / _points_per_unit;
}

/**
 * Sets the factor by which the font is rendered larger by the FreeType
 * library before being filtered down to its actual size in the texture as
 * specified by set_pixels_per_unit().  This may be set to a number larger
 * than 1.0 to improve the font's antialiasing (since FreeType doesn't really
 * do a swell job of antialiasing by itself).  There is some performance
 * implication for setting this different than 1.0.
 *
 * This should only be called before any characters have been requested out of
 * the font, or immediately after calling clear().
 */
INLINE bool FreetypeFont::
set_scale_factor(PN_stdfloat scale_factor) {
  _requested_scale_factor = scale_factor;
  return reset_scale();
}

/**
 * Returns the antialiasing scale factor.  See set_scale_factor().
 */
INLINE PN_stdfloat FreetypeFont::
get_scale_factor() const {
  return _scale_factor;
}

/**
 * Sets whether the Freetype library's built-in antialias mode is enabled.
 * There are two unrelated ways to achieve antialiasing: with Freetype's
 * native antialias mode, and with the use of a scale_factor greater than one.
 * By default, both modes are enabled.
 *
 * At low resolutions, some fonts may do better with one mode or the other.
 * In general, Freetype's native antialiasing will produce less blurry
 * results, but may introduce more artifacts.
 */
INLINE void FreetypeFont::
set_native_antialias(bool native_antialias) {
  _native_antialias = native_antialias;
}

/**
 * Returns whether Freetype's built-in antialias mode is enabled.  See
 * set_native_antialias().
 */
INLINE bool FreetypeFont::
get_native_antialias() const {
  return _native_antialias;
}

/**
 * This is used to report whether the requested pixel size is being only
 * approximated by a fixed-pixel-size font.  This returns 0 in the normal
 * case, in which a scalable font is used, or the fixed-pixel-size font has
 * exactly the requested pixel size.
 *
 * If this returns non-zero, it is the pixel size of the font that we are
 * using to approximate our desired size.
 */
INLINE int FreetypeFont::
get_font_pixel_size() const {
  return _pixel_height;
}

/**
 * Returns the number of units high each line of text is.
 */
INLINE PN_stdfloat FreetypeFont::
get_line_height() const {
  return _line_height;
}

/**
 * Returns the number of units wide a space is.
 */
INLINE PN_stdfloat FreetypeFont::
get_space_advance() const {
  return _space_advance;
}

/**
 * Returns the point size of the font that is one Panda unit high.  This is an
 * arbitrary Panda convention for text, and is set to 10.0.
 */
INLINE PN_stdfloat FreetypeFont::
get_points_per_unit() {
  return _points_per_unit;
}

/**
 * Returns the number of points in one inch.  This is a universal typographic
 * convention.
 */
INLINE PN_stdfloat FreetypeFont::
get_points_per_inch() {
  return _points_per_inch;
}

/**
 * Specifies an explicitly winding order on this particular font.  This is
 * only necessary if the render_mode is RM_polygon or RM_solid, and only if
 * FreeType appears to guess wrong on this font.  Normally, you should leave
 * this at WO_default.
 */
INLINE void FreetypeFont::
set_winding_order(WindingOrder winding_order) {
  _winding_order = winding_order;
}

/**
 * Returns the winding order set via set_winding_order().
 */
INLINE FreetypeFont::WindingOrder FreetypeFont::
get_winding_order() const {
  return _winding_order;
}

/**
 * Retrieves the internal freetype face, and also acquires the lock.
 *
 * You must call release_face() when you are done using it, to release the
 * lock.
 */
INLINE FT_Face FreetypeFont::
acquire_face() const {
  nassertr(_face != nullptr, nullptr);
  return _face->acquire_face(_char_size, _dpi, _pixel_width, _pixel_height);
}

/**
 * Releases the lock acquired by a previous call to acquire_face(), and allows
 * another thread to use the face.
 */
INLINE void FreetypeFont::
release_face(FT_Face face) const {
  nassertv(_face != nullptr);
  _face->release_face(face);
}

/**
 *
 */
INLINE FreetypeFont::ContourPoint::
ContourPoint(const LPoint2 &p, const LVector2 &in, const LVector2 &out) :
  _p(p), _in(in), _out(out), _radius(0)
{
}

/**
 *
 */
INLINE FreetypeFont::ContourPoint::
ContourPoint(PN_stdfloat px, PN_stdfloat py, PN_stdfloat tx, PN_stdfloat ty) :
  _p(px, py), _in(tx, ty), _out(tx, ty), _radius(0)
{
}

/**
 * Connects the indicated point to the next point, whose tangent is given.
 * The given tangent becomes the out tangent at this point.  If the in tangent
 * and out tangent are sufficiently close, they will be smoothed together.
 */
INLINE void FreetypeFont::ContourPoint::
connect_to(const LVector2 &out) {
  _out = out;
  if (_in.dot(_out) > 0.7071) {
    // Less than 45 degrees of difference: smooth them.
    LVector2 av = (_in + _out) * 0.5f;
    av.normalize();
    _in = av;
    _out = av;
  }
}
