/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file textProperties.I
 * @author drose
 * @date 2004-04-06
 */

/**
 *
 */
INLINE bool TextProperties::
operator != (const TextProperties &other) const {
  return !operator == (other);
}

/**
 * Returns true if any properties have been specified, false otherwise.
 */
INLINE bool TextProperties::
is_any_specified() const {
  return (_specified != 0);
}

/**
 * Specifies the default font to be used for any TextNode whose font is
 * uninitialized or NULL.  See set_font().
 */
INLINE void TextProperties::
set_default_font(TextFont *font) {
  // If the user overrides the default, we don't need to try to load whatever
  // it would have been.
  _loaded_default_font = true;
  _default_font = font;
}

/**
 * Specifies the default font to be used for any TextNode whose font is
 * uninitialized or NULL.  See set_font().
 */
INLINE TextFont *TextProperties::
get_default_font() {
  if (!_loaded_default_font) {
    load_default_font();
  }
  return _default_font;
}

/**
 * Sets the font that will be used when making text.  If this is set to NULL,
 * the default font will be used, which can be set via set_default_font().
 */
INLINE void TextProperties::
set_font(TextFont *font) {
  _font = font;
  _specified |= F_has_font;
}

/**
 * Restores the default font to the text.
 */
INLINE void TextProperties::
clear_font() {
  _font.clear();
  _specified &= ~F_has_font;
}

/**
 *
 */
INLINE bool TextProperties::
has_font() const {
  return (_specified & F_has_font) != 0;
}

/**
 * Returns the font currently in use, if any.  If no font is in use, this
 * returns the default font.
 */
INLINE TextFont *TextProperties::
get_font() const {
  return has_font() ? _font.p() : get_default_font();
}

/**
 * Sets the small_caps flag.  When this is set, lowercase letters are
 * generated as scaled-down versions of their uppercase equivalents.  This is
 * particularly useful to set for fonts that do not have lowercase letters.
 *
 * It is also a good idea to set this for a (dynamic) font that has already
 * implemented lowercase letters as scaled-down versions of their uppercase
 * equivalents, since without this flag the texture memory may needlessly
 * duplicate equivalent glyphs for upper and lowercase letters.  Setting this
 * flag causes the texture memory to share the mixed-case letters.
 *
 * The amount by which the lowercase letters are scaled is specified by
 * set_small_caps_scale().
 */
INLINE void TextProperties::
set_small_caps(bool small_caps) {
  _small_caps = small_caps;
  _specified |= F_has_small_caps;
}

/**
 *
 */
INLINE void TextProperties::
clear_small_caps() {
  _small_caps = false;
  _specified &= ~F_has_small_caps;
}

/**
 *
 */
INLINE bool TextProperties::
has_small_caps() const {
  return (_specified & F_has_small_caps) != 0;
}

/**
 * Returns the small_caps flag.  See set_small_caps().
 */
INLINE bool TextProperties::
get_small_caps() const {
  return _small_caps;
}

/**
 * Sets the scale factor applied to lowercase letters from their uppercase
 * equivalents, when the small_caps flag is in effect.  See set_small_caps().
 * Normally, this will be a number less than one.
 */
INLINE void TextProperties::
set_small_caps_scale(PN_stdfloat small_caps_scale) {
  _small_caps_scale = small_caps_scale;
}

/**
 *
 */
INLINE void TextProperties::
clear_small_caps_scale() {
  _small_caps_scale = text_small_caps_scale;
  _specified &= ~F_has_small_caps_scale;
}

/**
 *
 */
INLINE bool TextProperties::
has_small_caps_scale() const {
  return (_specified & F_has_small_caps_scale) != 0;
}

/**
 * Returns the scale factor applied to lowercase letters from their uppercase
 * equivalents, when the small_caps flag is in effect.  See set_small_caps()
 * and set_small_caps_scale().
 */
INLINE PN_stdfloat TextProperties::
get_small_caps_scale() const {
  return _small_caps_scale;
}

/**
 * Specifies the factor by which the text slants to the right.
 */
INLINE void TextProperties::
set_slant(PN_stdfloat slant) {
  _slant = slant;
  _specified |= F_has_slant;
}

/**
 *
 */
INLINE void TextProperties::
clear_slant() {
  _slant = 0.0f;
  _specified &= ~F_has_slant;
}

/**
 *
 */
INLINE bool TextProperties::
has_slant() const {
  return (_specified & F_has_slant) != 0;
}

/**
 * Returns the factor by which the text is specified to slant to the right.
 */
INLINE PN_stdfloat TextProperties::
get_slant() const {
  return _slant;
}

/**
 * Sets the underscore flag.  When this is set, the text is underscored with a
 * one-pixel line the same color as the text foreground, drawn at the
 * baseline.
 */
INLINE void TextProperties::
set_underscore(bool underscore) {
  _underscore = underscore;
  _specified |= F_has_underscore;
}

/**
 *
 */
INLINE void TextProperties::
clear_underscore() {
  _underscore = false;
  _specified &= ~F_has_underscore;
}

/**
 *
 */
INLINE bool TextProperties::
has_underscore() const {
  return (_specified & F_has_underscore) != 0;
}

/**
 * Returns the underscore flag.  See set_underscore().
 */
INLINE bool TextProperties::
get_underscore() const {
  return _underscore;
}

/**
 * Specifies the vertical height of the underscore, relative to the text
 * baseline.  This only has meaning if the underscore mode is enabled with
 * set_underscore().
 */
INLINE void TextProperties::
set_underscore_height(PN_stdfloat underscore_height) {
  _underscore_height = underscore_height;
  _specified |= F_has_underscore_height;
}

/**
 *
 */
INLINE void TextProperties::
clear_underscore_height() {
  _underscore_height = 0.0f;
  _specified &= ~F_has_underscore_height;
}

/**
 *
 */
INLINE bool TextProperties::
has_underscore_height() const {
  return (_specified & F_has_underscore_height) != 0;
}

/**
 * Returns the vertical height of the underscore; see set_underscore_height().
 */
INLINE PN_stdfloat TextProperties::
get_underscore_height() const {
  return has_underscore_height() ? _underscore_height : text_default_underscore_height;
}

/**
 * Specifies the alignment of the text within its margins.
 */
INLINE void TextProperties::
set_align(TextProperties::Alignment align_type) {
  _align = align_type;
  _specified |= F_has_align;
}

/**
 * Restores the default alignment of the text.
 */
INLINE void TextProperties::
clear_align() {
  _align = A_left;
  _specified &= ~F_has_align;
}

/**
 *
 */
INLINE bool TextProperties::
has_align() const {
  return (_specified & F_has_align) != 0;
}

/**
 *
 */
INLINE TextProperties::Alignment TextProperties::
get_align() const {
  return _align;
}

/**
 * Specifies the amount of extra space that is inserted before the first
 * character of each line.  This can be thought of as a left margin.
 */
INLINE void TextProperties::
set_indent(PN_stdfloat indent) {
  _indent_width = indent;
  _specified |= F_has_indent;
}

/**
 * Removes the indent setting from the text.  Text will be as wide as it is.
 */
INLINE void TextProperties::
clear_indent() {
  _indent_width = 0.0f;
  _specified &= ~F_has_indent;
}

/**
 *
 */
INLINE bool TextProperties::
has_indent() const {
  return (_specified & F_has_indent) != 0;
}

/**
 *
 */
INLINE PN_stdfloat TextProperties::
get_indent() const {
  return _indent_width;
}

/**
 * Sets the text up to automatically wordwrap when it exceeds the indicated
 * width.  This can be thought of as a right margin or margin width.
 */
INLINE void TextProperties::
set_wordwrap(PN_stdfloat wordwrap) {
  _wordwrap_width = wordwrap;
  _specified |= F_has_wordwrap;
}

/**
 * Removes the wordwrap setting from the text.  Text will be as wide as it is.
 */
INLINE void TextProperties::
clear_wordwrap() {
  _wordwrap_width = 0.0f;
  _specified &= ~F_has_wordwrap;
}

/**
 *
 */
INLINE bool TextProperties::
has_wordwrap() const {
  return (_specified & F_has_wordwrap) != 0;
}

/**
 *
 */
INLINE PN_stdfloat TextProperties::
get_wordwrap() const {
  return _wordwrap_width;
}

/**
 * Sets the preserve_trailing_whitespace flag.  When this is set, trailing
 * whitespace at the end of the line is not stripped when the text is
 * wordwrapped (it is stripped by default).  Since the trailing whitespace is
 * invisible, this is important primarily for determining the proper width of
 * a frame or card behind the text.
 */
INLINE void TextProperties::
set_preserve_trailing_whitespace(bool preserve_trailing_whitespace) {
  _preserve_trailing_whitespace = preserve_trailing_whitespace;
  _specified |= F_has_preserve_trailing_whitespace;
}

/**
 *
 */
INLINE void TextProperties::
clear_preserve_trailing_whitespace() {
  _preserve_trailing_whitespace = false;
  _specified &= ~F_has_preserve_trailing_whitespace;
}

/**
 *
 */
INLINE bool TextProperties::
has_preserve_trailing_whitespace() const {
  return (_specified & F_has_preserve_trailing_whitespace) != 0;
}

/**
 * Returns the preserve_trailing_whitespace flag.  See
 * set_preserve_trailing_whitespace().
 */
INLINE bool TextProperties::
get_preserve_trailing_whitespace() const {
  return _preserve_trailing_whitespace;
}

/**
 *
 */
INLINE void TextProperties::
set_text_color(PN_stdfloat r, PN_stdfloat g, PN_stdfloat b, PN_stdfloat a) {
  set_text_color(LColor(r, g, b, a));
}

/**
 *
 */
INLINE void TextProperties::
set_text_color(const LColor &text_color) {
  _text_color = text_color;
  _specified |= F_has_text_color;
  _text_state.clear();
}

/**
 * Removes the text color specification; the text will be colored whatever it
 * was in the source font file.
 */
INLINE void TextProperties::
clear_text_color() {
  _text_color.set(1.0f, 1.0f, 1.0f, 1.0f);
  _specified &= ~F_has_text_color;
  _text_state.clear();
}

/**
 *
 */
INLINE bool TextProperties::
has_text_color() const {
  return (_specified & F_has_text_color) != 0;
}

/**
 *
 */
INLINE LColor TextProperties::
get_text_color() const {
  return _text_color;
}

/**
 *
 */
INLINE void TextProperties::
set_shadow_color(PN_stdfloat r, PN_stdfloat g, PN_stdfloat b, PN_stdfloat a) {
  set_shadow_color(LColor(r, g, b, a));
}

/**
 *
 */
INLINE void TextProperties::
set_shadow_color(const LColor &shadow_color) {
  _shadow_color = shadow_color;
  _specified |= F_has_shadow_color;
  _shadow_state.clear();
}

/**
 * Removes the shadow color specification.
 */
INLINE void TextProperties::
clear_shadow_color() {
  _shadow_color.set(0.0f, 0.0f, 0.0f, 1.0f);
  _specified &= ~F_has_shadow_color;
  _shadow_state.clear();
}

/**
 *
 */
INLINE bool TextProperties::
has_shadow_color() const {
  return (_specified & F_has_shadow_color) != 0;
}

/**
 *
 */
INLINE LColor TextProperties::
get_shadow_color() const {
  return _shadow_color;
}

/**
 * Specifies that the text should be drawn with a shadow, by creating a second
 * copy of the text and offsetting it slightly behind the first.
 */
INLINE void TextProperties::
set_shadow(PN_stdfloat xoffset, PN_stdfloat yoffset) {
  set_shadow(LVecBase2(xoffset, yoffset));
}

/**
 * Specifies that the text should be drawn with a shadow, by creating a second
 * copy of the text and offsetting it slightly behind the first.
 */
INLINE void TextProperties::
set_shadow(const LVecBase2 &shadow_offset) {
  _shadow_offset = shadow_offset;
  _specified |= F_has_shadow;
}

/**
 * Specifies that a shadow will not be drawn behind the text.
 */
INLINE void TextProperties::
clear_shadow() {
  _specified &= ~F_has_shadow;
  _shadow_offset.set(0.0f, 0.0f);
}

/**
 *
 */
INLINE bool TextProperties::
has_shadow() const {
  return (_specified & F_has_shadow) != 0;
}

/**
 * Returns the offset of the shadow as set by set_shadow().  It is an error to
 * call this if has_shadow() is false.
 */
INLINE LVector2 TextProperties::
get_shadow() const {
  return _shadow_offset;
}

/**
 * Names the CullBin that the text geometry should be assigned to.  If this is
 * set, then a CullBinAttrib will be created to explicitly place each
 * component in the named bin.
 *
 * The draw_order value will also be passed to each CullBinAttrib as
 * appropriate; this is particularly useful if this names a CullBinFixed, e.g.
 * "fixed".
 */
INLINE void TextProperties::
set_bin(const std::string &bin) {
  _bin = bin;
  _specified |= F_has_bin;
  _text_state.clear();
  _shadow_state.clear();
}

/**
 * Removes the effect of a previous call to set_bin().  Text will be drawn in
 * whatever bin it would like to be drawn in, with no explicit ordering.
 */
INLINE void TextProperties::
clear_bin() {
  _bin = std::string();
  _specified &= ~F_has_bin;
  _text_state.clear();
  _shadow_state.clear();
}

/**
 * Returns true if an explicit drawing bin has been set via set_bin(), false
 * otherwise.
 */
INLINE bool TextProperties::
has_bin() const {
  return (_specified & F_has_bin) != 0;
}

/**
 * Returns the drawing bin set with set_bin(), or empty string if no bin has
 * been set.
 */
INLINE const std::string &TextProperties::
get_bin() const {
  return _bin;
}

/**
 * Sets the drawing order of text created by the TextNode.  This is actually
 * the draw order of the card and frame.  The shadow is drawn at
 * _draw_order+1, and the text at _draw_order+2.
 *
 * This affects the sorting order assigned to the nodes as they are created,
 * and also is passed to whatever bin may be assigned via set_bin().
 *
 * The return value is the first unused draw_order number, e.g.  _draw_order +
 * 3.
 */
INLINE int TextProperties::
set_draw_order(int draw_order) {
  _draw_order = draw_order;
  _specified |= F_has_draw_order;
  _text_state.clear();
  _shadow_state.clear();
  return _draw_order + 3;
}

/**
 *
 */
INLINE void TextProperties::
clear_draw_order() {
  _draw_order = 1;
  _specified &= ~F_has_draw_order;
  _text_state.clear();
  _shadow_state.clear();
}

/**
 *
 */
INLINE bool TextProperties::
has_draw_order() const {
  return (_specified & F_has_draw_order) != 0;
}

/**
 * Returns the drawing order set with set_draw_order().
 */
INLINE int TextProperties::
get_draw_order() const {
  return _draw_order;
}

/**
 * Sets the width of each tab stop, in screen units.  A tab character embedded
 * in the text will advance the horizontal position to the next tab stop.
 */
INLINE void TextProperties::
set_tab_width(PN_stdfloat tab_width) {
  _tab_width = tab_width;
  _specified |= F_has_tab_width;
}

/**
 *
 */
INLINE void TextProperties::
clear_tab_width() {
  _tab_width = text_tab_width;
  _specified &= ~F_has_tab_width;
}

/**
 *
 */
INLINE bool TextProperties::
has_tab_width() const {
  return (_specified & F_has_tab_width) != 0;
}

/**
 * Returns the width set via set_tab_width().
 */
INLINE PN_stdfloat TextProperties::
get_tab_width() const {
  return _tab_width;
}

/**
 * Specifies the factor by which to scale each letter of the text as it is
 * placed, in addition to any scales inherited from the node or from
 * set_text_scale(). This can be used (possibly in conjunction with
 * set_glyph_shift()) to implement superscripting or subscripting.
 *
 * The glyph scale is cumulative when applied to nested TextProperties.  It is
 * intended primarily for implementing superscripts, not for scaling the text
 * in general.  See also set_text_scale(), which is intended primarily for
 * scaling the text in general, and is not cumulative.
 */
INLINE void TextProperties::
set_glyph_scale(PN_stdfloat glyph_scale) {
  _glyph_scale = glyph_scale;
  _specified |= F_has_glyph_scale;
}

/**
 *
 */
INLINE void TextProperties::
clear_glyph_scale() {
  _specified &= ~F_has_glyph_scale;
  _glyph_scale = 0.0f;
}

/**
 *
 */
INLINE bool TextProperties::
has_glyph_scale() const {
  return (_specified & F_has_glyph_scale) != 0;
}

/**
 * Returns the scale factor of each letter as specified by set_glyph_scale().
 */
INLINE PN_stdfloat TextProperties::
get_glyph_scale() const {
  return _glyph_scale;
}

/**
 * Specifies a vertical amount to shift each letter of the text as it is
 * placed.  This can be used (possibly in conjunction with set_glyph_scale())
 * to implement superscripting or subscripting.
 */
INLINE void TextProperties::
set_glyph_shift(PN_stdfloat glyph_shift) {
  _glyph_shift = glyph_shift;
  _specified |= F_has_glyph_shift;
}

/**
 *
 */
INLINE void TextProperties::
clear_glyph_shift() {
  _specified &= ~F_has_glyph_shift;
  _glyph_shift = 0.0f;
}

/**
 *
 */
INLINE bool TextProperties::
has_glyph_shift() const {
  return (_specified & F_has_glyph_shift) != 0;
}

/**
 * Returns the vertical shift of each letter as specified by
 * set_glyph_shift().
 */
INLINE PN_stdfloat TextProperties::
get_glyph_shift() const {
  return _glyph_shift;
}

/**
 * Specifies the factor by which to scale the text, in addition to any
 * scalings imposed by the node, as well as in addition to the glyph scale.
 *
 * The text scale is not cumulative when applied to nested TextProperties.
 * See also set_glyph_scale(), which is cumulative.
 */
INLINE void TextProperties::
set_text_scale(PN_stdfloat text_scale) {
  _text_scale = text_scale;
  _specified |= F_has_text_scale;
}

/**
 *
 */
INLINE void TextProperties::
clear_text_scale() {
  _specified &= ~F_has_text_scale;
  _text_scale = 0.0f;
}

/**
 *
 */
INLINE bool TextProperties::
has_text_scale() const {
  return (_specified & F_has_text_scale) != 0;
}

/**
 * Returns the scale factor of the text as specified by set_text_scale().
 */
INLINE PN_stdfloat TextProperties::
get_text_scale() const {
  return _text_scale;
}

/**
 * Specifies the text direction.  If none is specified, it will be guessed
 * based on the contents of the string.
 *
 * @since 1.10.0
 */
INLINE void TextProperties::
set_direction(Direction direction) {
  _direction = direction;
  _specified |= F_has_direction;
}

/**
 * Clears the text direction setting.  If no text direction is specified, it
 * will be guessed based on the contents of the string.
 *
 * @since 1.10.0
 */
INLINE void TextProperties::
clear_direction() {
  _specified &= ~F_has_direction;
  _direction = D_ltr;
}

/**
 * @since 1.10.0
 */
INLINE bool TextProperties::
has_direction() const {
  return (_specified & F_has_direction) != 0;
}

/**
 * Returns the direction of the text as specified by set_direction().
 *
 * @since 1.10.0
 */
INLINE TextProperties::Direction TextProperties::
get_direction() const {
  return _direction;
}
