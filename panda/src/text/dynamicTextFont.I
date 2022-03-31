/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file dynamicTextFont.I
 * @author drose
 * @date 2002-02-08
 */

/**
 * Disambiguates the get_name() method between that inherited from TextFont
 * and that inherited from FreetypeFont.
 */
INLINE const std::string &DynamicTextFont::
get_name() const {
  return TextFont::get_name();
}

/**
 * Sets the point size of the font.  This controls the apparent size of the
 * font onscreen.  By convention, a 10 point font is about 1 screen unit high.
 *
 * This should only be called before any characters have been requested out of
 * the font, or immediately after calling clear().
 */
INLINE bool DynamicTextFont::
set_point_size(PN_stdfloat point_size) {
  // If this assertion fails, you didn't call clear() first.  RTFM.
  nassertr(get_num_pages() == 0, false);

  return FreetypeFont::set_point_size(point_size);
}

/**
 * Returns the point size of the font.
 */
INLINE PN_stdfloat DynamicTextFont::
get_point_size() const {
  return FreetypeFont::get_point_size();
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
INLINE bool DynamicTextFont::
set_pixels_per_unit(PN_stdfloat pixels_per_unit) {
  // If this assertion fails, you didn't call clear() first.  RTFM.
  nassertr(get_num_pages() == 0, false);

  return FreetypeFont::set_pixels_per_unit(pixels_per_unit);
}

/**
 * Returns the resolution of the texture map.  See set_pixels_per_unit().
 */
INLINE PN_stdfloat DynamicTextFont::
get_pixels_per_unit() const {
  return FreetypeFont::get_pixels_per_unit();
}

/**
 * Sets the factor by which the font is rendered larger by the FreeType
 * library before being filtered down to its actual size in the texture as
 * specified by set_pixels_per_unit().  This may be set to a number larger
 * than 1.0 to improve the font's antialiasing (since FreeType doesn't really
 * do a swell job of antialiasing by itself).  There is some performance
 * implication for setting this different than 1.0, but it is probably small.
 *
 * This should only be called before any characters have been requested out of
 * the font, or immediately after calling clear().
 */
INLINE bool DynamicTextFont::
set_scale_factor(PN_stdfloat scale_factor) {
  // If this assertion fails, you didn't call clear() first.  RTFM.
  nassertr(get_num_pages() == 0, false);

  return FreetypeFont::set_scale_factor(scale_factor);
}

/**
 * Returns the antialiasing scale factor.  See set_scale_factor().
 */
INLINE PN_stdfloat DynamicTextFont::
get_scale_factor() const {
  return FreetypeFont::get_scale_factor();
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
INLINE void DynamicTextFont::
set_native_antialias(bool native_antialias) {
  // If this assertion fails, you didn't call clear() first.  RTFM.
  nassertv(get_num_pages() == 0);

  FreetypeFont::set_native_antialias(native_antialias);
}

/**
 * Returns whether Freetype's built-in antialias mode is enabled.  See
 * set_native_antialias().
 */
INLINE bool DynamicTextFont::
get_native_antialias() const {
  return FreetypeFont::get_native_antialias();
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
INLINE int DynamicTextFont::
get_font_pixel_size() const {
  return FreetypeFont::get_font_pixel_size();
}

/**
 * Returns the number of units high each line of text is.
 */
INLINE PN_stdfloat DynamicTextFont::
get_line_height() const {
  return TextFont::get_line_height();
}

/**
 * Returns the number of units wide a space is.
 */
INLINE PN_stdfloat DynamicTextFont::
get_space_advance() const {
  return TextFont::get_space_advance();
}

/**
 * Sets the number of pixels of padding that is added around the border of
 * each glyph before adding it to the texture map.  This reduces the bleed in
 * from neighboring glyphs in the texture map.
 */
INLINE void DynamicTextFont::
set_texture_margin(int texture_margin) {
  _texture_margin = texture_margin;
}

/**
 * Returns the number of pixels of padding that is added around the border of
 * each glyph in the texture map.  See set_texture_margin().
 */
INLINE int DynamicTextFont::
get_texture_margin() const {
  return _texture_margin;
}

/**
 * Sets the number of pixels of padding that is included around each glyph in
 * the generated polygons.  This helps prevent the edges of the glyphs from
 * being cut off at small minifications.  It is not related to the amount of
 * extra pixels reserved in the texture map (but it should be set somewhat
 * smaller than this number, which is controlled by set_texture_margin(), to
 * prevent bleed-in from neighboring letters in the texture).
 */
INLINE void DynamicTextFont::
set_poly_margin(PN_stdfloat poly_margin) {
  _poly_margin = poly_margin;
}

/**
 * Returns the number of pixels of padding that is included around each glyph
 * in the generated polygons.  See set_poly_margin().
 */
INLINE PN_stdfloat DynamicTextFont::
get_poly_margin() const {
  return _poly_margin;
}

/**
 * Sets the x, y size of the textures that are created for the
 * DynamicTextFont.
 */
INLINE void DynamicTextFont::
set_page_size(const LVecBase2i &page_size) {
  _page_size = page_size;
}

/**
 * Sets the x, y size of the textures that are created for the
 * DynamicTextFont.
 */
INLINE void DynamicTextFont::
set_page_size(int x_size, int y_size) {
  _page_size.set(x_size, y_size);
}

/**
 * Returns the size of the textures that are created for the DynamicTextFont.
 * See set_page_size().
 */
INLINE const LVecBase2i &DynamicTextFont::
get_page_size() const {
  return _page_size;
}

/**
 * Returns the x size of the textures that are created for the
 * DynamicTextFont.  See set_page_size().
 */
INLINE int DynamicTextFont::
get_page_x_size() const {
  return _page_size.get_x();
}

/**
 * Returns the y size of the textures that are created for the
 * DynamicTextFont.  See set_page_size().
 */
INLINE int DynamicTextFont::
get_page_y_size() const {
  return _page_size.get_y();
}

/**
 * Sets the filter type used when minimizing the textures created for this
 * font.
 */
INLINE void DynamicTextFont::
set_minfilter(SamplerState::FilterType filter) {
  _minfilter = filter;
  update_filters();
}

/**
 * Returns the filter type used when minimizing the textures created for this
 * font.
 */
INLINE SamplerState::FilterType DynamicTextFont::
get_minfilter() const {
  return _minfilter;
}

/**
 * Sets the filter type used when enlarging the textures created for this
 * font.
 */
INLINE void DynamicTextFont::
set_magfilter(SamplerState::FilterType filter) {
  _magfilter = filter;
  update_filters();
}

/**
 * Returns the filter type used when enlarging the textures created for this
 * font.
 */
INLINE SamplerState::FilterType DynamicTextFont::
get_magfilter() const {
  return _magfilter;
}

/**
 * Enables or disables anisotropic filtering on the textures created for this
 * font.  The default value is specified by the text-anisotropic-degree
 * variable.  See Texture::set_anisotropic_degree().
 */
INLINE void DynamicTextFont::
set_anisotropic_degree(int anisotropic_degree) {
  _anisotropic_degree = anisotropic_degree;
  update_filters();
}

/**
 * Returns the current anisotropic degree for textures created for this font.
 * See set_anisotropic_degree().
 */
INLINE int DynamicTextFont::
get_anisotropic_degree() const {
  return _anisotropic_degree;
}

/**
 * Specifies the way the glyphs on this particular font are generated.  The
 * default is RM_texture, which is the only mode supported for bitmap fonts.
 * Other modes are possible for most modern fonts.
 */
INLINE void DynamicTextFont::
set_render_mode(DynamicTextFont::RenderMode render_mode) {
  _render_mode = render_mode;
}

/**
 * Returns the way the glyphs on this particular font are generated.  See
 * set_render_mode().
 */
INLINE DynamicTextFont::RenderMode DynamicTextFont::
get_render_mode() const {
  return _render_mode;
}

/**
 * Changes the color of the foreground pixels of the font as they are rendered
 * into the font texture.  The default is (1, 1, 1, 1), or opaque white, which
 * allows text created with the font to be colored individually.  Normally,
 * you would not change this unless you really need a particular color effect
 * to appear in the font itself.
 *
 * This should only be called before any characters have been requested out of
 * the font, or immediately after calling clear().
 */
INLINE void DynamicTextFont::
set_fg(const LColor &fg) {
  // If this assertion fails, you didn't call clear() first.  RTFM.
  nassertv(get_num_pages() == 0);

  _fg = fg;
  determine_tex_format();
}

/**
 * Returns the color of the foreground pixels of the font as they are rendered
 * into the font texture.  See set_fg().
 */
INLINE const LColor &DynamicTextFont::
get_fg() const {
  return _fg;
}

/**
 * Changes the color of the background pixels of the font as they are rendered
 * into the font texture.  The default is (1, 1, 1, 0), or transparent white,
 * which allows text created with the font to be colored individually.  (Note
 * that it should not generally be (0, 0, 0, 0), which would tend to bleed
 * into the foreground color, unless you have also specified a outline color
 * of (0, 0, 0, 1)) .
 *
 * Normally, you would not change this unless you really need a particular
 * color effect to appear in the font itself.
 *
 * This should only be called before any characters have been requested out of
 * the font, or immediately after calling clear().
 */
INLINE void DynamicTextFont::
set_bg(const LColor &bg) {
  // If this assertion fails, you didn't call clear() first.  RTFM.
  nassertv(get_num_pages() == 0);

  _bg = bg;
  determine_tex_format();
}

/**
 * Returns the color of the background pixels of the font as they are rendered
 * into the font texture.  See set_bg().
 */
INLINE const LColor &DynamicTextFont::
get_bg() const {
  return _bg;
}


/**
 * Sets up the font to have an outline around each font letter.  This is
 * achieved via a Gaussian post-process as each letter is generated; there is
 * some runtime cost for this effect, but it is minimal as each letter is
 * normally generated only once and then cached.
 *
 * The color is the desired color of the outline, width is the number of
 * points beyond the letter that the outline extends (a typical font is 10
 * points high), and feather is a number in the range 0.0 .. 1.0 that controls
 * the softness of the outline.  Set the width to 0.0 to disable the outline.
 *
 * This should only be called before any characters have been requested out of
 * the font, or immediately after calling clear().
 */
INLINE void DynamicTextFont::
set_outline(const LColor &outline_color, PN_stdfloat outline_width,
            PN_stdfloat outline_feather) {
  // If this assertion fails, you didn't call clear() first.  RTFM.
  nassertv(get_num_pages() == 0);

  _outline_color = outline_color;
  _outline_width = outline_width;
  _outline_feather = outline_feather;
  determine_tex_format();
}

/**
 * Returns the color of the outline pixels of the font as they are rendered
 * into the font texture.  See set_outline().
 */
INLINE const LColor &DynamicTextFont::
get_outline_color() const {
  return _outline_color;
}

/**
 * Returns the width of the outline pixels of the font, as the number of
 * points beyond each letter.  See set_outline().
 */
INLINE PN_stdfloat DynamicTextFont::
get_outline_width() const {
  return _outline_width;
}

/**
 * Returns the softness of the outline pixels of the font, as a value in the
 * range 0.0 to 1.0. See set_outline().
 */
INLINE PN_stdfloat DynamicTextFont::
get_outline_feather() const {
  return _outline_feather;
}

/**
 * Returns the texture format used to render the individual pages.  This is
 * set automatically according to the colors selected.
 */
INLINE Texture::Format DynamicTextFont::
get_tex_format() const {
  return _tex_format;
}


INLINE std::ostream &
operator << (std::ostream &out, const DynamicTextFont &dtf) {
  return out << dtf.get_name();
}
