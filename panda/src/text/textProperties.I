// Filename: textProperties.I
// Created by:  drose (06Apr04)
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
//     Function: TextProperties::is_any_specified
//       Access: Published
//  Description: Returns true if any properties have been specified,
//               false otherwise.
////////////////////////////////////////////////////////////////////
INLINE bool TextProperties::
is_any_specified() const {
  return (_specified != 0);
}

////////////////////////////////////////////////////////////////////
//     Function: TextProperties::set_default_font
//       Access: Published, Static
//  Description: Specifies the default font to be used for any
//               TextNode whose font is uninitialized or NULL.  See
//               set_font().
////////////////////////////////////////////////////////////////////
INLINE void TextProperties::
set_default_font(TextFont *font) {
  // If the user overrides the default, we don't need to try to load
  // whatever it would have been.
  _loaded_default_font = true;
  _default_font = font;
}

////////////////////////////////////////////////////////////////////
//     Function: TextProperties::get_default_font
//       Access: Published, Static
//  Description: Specifies the default font to be used for any
//               TextNode whose font is uninitialized or NULL.  See
//               set_font().
////////////////////////////////////////////////////////////////////
INLINE TextFont *TextProperties::
get_default_font() {
  if (!_loaded_default_font) {
    load_default_font();
  }
  return _default_font;
}

////////////////////////////////////////////////////////////////////
//     Function: TextProperties::set_font
//       Access: Published
//  Description: Sets the font that will be used when making text.  If
//               this is set to NULL, the default font will be used,
//               which can be set via set_default_font().
////////////////////////////////////////////////////////////////////
INLINE void TextProperties::
set_font(TextFont *font) {
  _font = font;
  _specified |= F_has_font;
}

////////////////////////////////////////////////////////////////////
//     Function: TextProperties::clear_font
//       Access: Published
//  Description: Restores the default font to the text.
////////////////////////////////////////////////////////////////////
INLINE void TextProperties::
clear_font() {
  _font.clear();
  _specified &= ~F_has_font;
}

////////////////////////////////////////////////////////////////////
//     Function: TextProperties::has_font
//       Access: Published
//  Description:
////////////////////////////////////////////////////////////////////
INLINE bool TextProperties::
has_font() const {
  return (_specified & F_has_font) != 0;
}

////////////////////////////////////////////////////////////////////
//     Function: TextProperties::get_font
//       Access: Published
//  Description: Returns the font currently in use, if any.  If no
//               font is in use, this returns the default font.
////////////////////////////////////////////////////////////////////
INLINE TextFont *TextProperties::
get_font() const {
  return has_font() ? _font.p() : get_default_font();
}

////////////////////////////////////////////////////////////////////
//     Function: TextProperties::set_small_caps
//       Access: Published
//  Description: Sets the small_caps flag.  When this is set,
//               lowercase letters are generated as scaled-down
//               versions of their uppercase equivalents.  This is
//               particularly useful to set for fonts that do not have
//               lowercase letters.
//
//               It is also a good idea to set this for a (dynamic)
//               font that has already implemented lowercase letters
//               as scaled-down versions of their uppercase
//               equivalents, since without this flag the texture
//               memory may needlessly duplicate equivalent glyphs for
//               upper and lowercase letters.  Setting this flag
//               causes the texture memory to share the mixed-case
//               letters.
//
//               The amount by which the lowercase letters are scaled
//               is specified by set_small_caps_scale().
////////////////////////////////////////////////////////////////////
INLINE void TextProperties::
set_small_caps(bool small_caps) {
  _small_caps = small_caps;
  _specified |= F_has_small_caps;
}

////////////////////////////////////////////////////////////////////
//     Function: TextProperties::clear_small_caps
//       Access: Published
//  Description:
////////////////////////////////////////////////////////////////////
INLINE void TextProperties::
clear_small_caps() {
  _small_caps = false;
  _specified &= ~F_has_small_caps;
}

////////////////////////////////////////////////////////////////////
//     Function: TextProperties::has_small_caps
//       Access: Published
//  Description:
////////////////////////////////////////////////////////////////////
INLINE bool TextProperties::
has_small_caps() const {
  return (_specified & F_has_small_caps) != 0;
}

////////////////////////////////////////////////////////////////////
//     Function: TextProperties::get_small_caps
//       Access: Published
//  Description: Returns the small_caps flag.  See set_small_caps().
////////////////////////////////////////////////////////////////////
INLINE bool TextProperties::
get_small_caps() const {
  return _small_caps;
}

////////////////////////////////////////////////////////////////////
//     Function: TextProperties::set_small_caps_scale
//       Access: Published
//  Description: Sets the scale factor applied to lowercase letters
//               from their uppercase equivalents, when the small_caps
//               flag is in effect.  See set_small_caps().  Normally,
//               this will be a number less than one.
////////////////////////////////////////////////////////////////////
INLINE void TextProperties::
set_small_caps_scale(float small_caps_scale) {
  _small_caps_scale = small_caps_scale;
}

////////////////////////////////////////////////////////////////////
//     Function: TextProperties::clear_small_caps_scale
//       Access: Published
//  Description:
////////////////////////////////////////////////////////////////////
INLINE void TextProperties::
clear_small_caps_scale() {
  _small_caps_scale = text_small_caps_scale;
  _specified &= ~F_has_small_caps_scale;
}

////////////////////////////////////////////////////////////////////
//     Function: TextProperties::has_small_caps_scale
//       Access: Published
//  Description:
////////////////////////////////////////////////////////////////////
INLINE bool TextProperties::
has_small_caps_scale() const {
  return (_specified & F_has_small_caps_scale) != 0;
}

////////////////////////////////////////////////////////////////////
//     Function: TextProperties::get_small_caps_scale
//       Access: Published
//  Description: Returns the scale factor applied to lowercase letters
//               from their uppercase equivalents, when the small_caps
//               flag is in effect.  See set_small_caps() and
//               set_small_caps_scale().
////////////////////////////////////////////////////////////////////
INLINE float TextProperties::
get_small_caps_scale() const {
  return _small_caps_scale;
}

////////////////////////////////////////////////////////////////////
//     Function: TextProperties::set_slant
//       Access: Published
//  Description: Specifies the factor by which the text slants to the
//               right.
////////////////////////////////////////////////////////////////////
INLINE void TextProperties::
set_slant(float slant) {
  _slant = slant;
  _specified |= F_has_slant;
}

////////////////////////////////////////////////////////////////////
//     Function: TextProperties::clear_slant
//       Access: Published
//  Description: 
////////////////////////////////////////////////////////////////////
INLINE void TextProperties::
clear_slant() {
  _slant = 0.0f;
  _specified &= ~F_has_slant;
}

////////////////////////////////////////////////////////////////////
//     Function: TextProperties::has_slant
//       Access: Published
//  Description:
////////////////////////////////////////////////////////////////////
INLINE bool TextProperties::
has_slant() const {
  return (_specified & F_has_slant) != 0;
}

////////////////////////////////////////////////////////////////////
//     Function: TextProperties::get_slant
//       Access: Published
//  Description: Returns the factor by which the text is specified to
//               slant to the right.
////////////////////////////////////////////////////////////////////
INLINE float TextProperties::
get_slant() const {
  return _slant;
}

////////////////////////////////////////////////////////////////////
//     Function: TextProperties::set_align
//       Access: Published
//  Description: Specifies the alignment of the text within its
//               margins.
////////////////////////////////////////////////////////////////////
INLINE void TextProperties::
set_align(TextProperties::Alignment align_type) {
  _align = align_type;
  _specified |= F_has_align;
}

////////////////////////////////////////////////////////////////////
//     Function: TextProperties::clear_align
//       Access: Published
//  Description: Restores the default alignment of the text.
////////////////////////////////////////////////////////////////////
INLINE void TextProperties::
clear_align() {
  _align = A_left;
  _specified &= ~F_has_align;
}

////////////////////////////////////////////////////////////////////
//     Function: TextProperties::has_align
//       Access: Published
//  Description:
////////////////////////////////////////////////////////////////////
INLINE bool TextProperties::
has_align() const {
  return (_specified & F_has_align) != 0;
}

////////////////////////////////////////////////////////////////////
//     Function: TextProperties::get_align
//       Access: Published
//  Description:
////////////////////////////////////////////////////////////////////
INLINE TextProperties::Alignment TextProperties::
get_align() const {
  return _align;
}

////////////////////////////////////////////////////////////////////
//     Function: TextProperties::set_indent
//       Access: Published
//  Description: Specifies the amount of extra space that is inserted
//               before the first character of each line.  This can be
//               thought of as a left margin.
////////////////////////////////////////////////////////////////////
INLINE void TextProperties::
set_indent(float indent) {
  _indent_width = indent;
  _specified |= F_has_indent;
}

////////////////////////////////////////////////////////////////////
//     Function: TextProperties::clear_indent
//       Access: Published
//  Description: Removes the indent setting from the text.  Text
//               will be as wide as it is.
////////////////////////////////////////////////////////////////////
INLINE void TextProperties::
clear_indent() {
  _indent_width = 0.0f;
  _specified &= ~F_has_indent;
}

////////////////////////////////////////////////////////////////////
//     Function: TextProperties::has_indent
//       Access: Published
//  Description:
////////////////////////////////////////////////////////////////////
INLINE bool TextProperties::
has_indent() const {
  return (_specified & F_has_indent) != 0;
}

////////////////////////////////////////////////////////////////////
//     Function: TextProperties::get_indent
//       Access: Published
//  Description:
////////////////////////////////////////////////////////////////////
INLINE float TextProperties::
get_indent() const {
  return _indent_width;
}

////////////////////////////////////////////////////////////////////
//     Function: TextProperties::set_wordwrap
//       Access: Published
//  Description: Sets the text up to automatically wordwrap when it
//               exceeds the indicated width.  This can be thought of
//               as a right margin or margin width.
////////////////////////////////////////////////////////////////////
INLINE void TextProperties::
set_wordwrap(float wordwrap) {
  _wordwrap_width = wordwrap;
  _specified |= F_has_wordwrap;
}

////////////////////////////////////////////////////////////////////
//     Function: TextProperties::clear_wordwrap
//       Access: Published
//  Description: Removes the wordwrap setting from the text.  Text
//               will be as wide as it is.
////////////////////////////////////////////////////////////////////
INLINE void TextProperties::
clear_wordwrap() {
  _wordwrap_width = 0.0f;
  _specified &= ~F_has_wordwrap;
}

////////////////////////////////////////////////////////////////////
//     Function: TextProperties::has_wordwrap
//       Access: Published
//  Description:
////////////////////////////////////////////////////////////////////
INLINE bool TextProperties::
has_wordwrap() const {
  return (_specified & F_has_wordwrap) != 0;
}

////////////////////////////////////////////////////////////////////
//     Function: TextProperties::get_wordwrap
//       Access: Published
//  Description:
////////////////////////////////////////////////////////////////////
INLINE float TextProperties::
get_wordwrap() const {
  return _wordwrap_width;
}

////////////////////////////////////////////////////////////////////
//     Function: TextProperties::set_preserve_trailing_whitespace
//       Access: Published
//  Description: Sets the preserve_trailing_whitespace flag.  When
//               this is set, trailing whitespace at the end of the
//               line is not stripped when the text is wordwrapped (it
//               is stripped by default).  Since the trailing
//               whitespace is invisible, this is important primarily
//               for determining the proper width of a frame or card
//               behind the text.
////////////////////////////////////////////////////////////////////
INLINE void TextProperties::
set_preserve_trailing_whitespace(bool preserve_trailing_whitespace) {
  _preserve_trailing_whitespace = preserve_trailing_whitespace;
  _specified |= F_has_preserve_trailing_whitespace;
}

////////////////////////////////////////////////////////////////////
//     Function: TextProperties::clear_preserve_trailing_whitespace
//       Access: Published
//  Description:
////////////////////////////////////////////////////////////////////
INLINE void TextProperties::
clear_preserve_trailing_whitespace() {
  _preserve_trailing_whitespace = false;
  _specified &= ~F_has_preserve_trailing_whitespace;
}

////////////////////////////////////////////////////////////////////
//     Function: TextProperties::has_preserve_trailing_whitespace
//       Access: Published
//  Description:
////////////////////////////////////////////////////////////////////
INLINE bool TextProperties::
has_preserve_trailing_whitespace() const {
  return (_specified & F_has_preserve_trailing_whitespace) != 0;
}

////////////////////////////////////////////////////////////////////
//     Function: TextProperties::get_preserve_trailing_whitespace
//       Access: Published
//  Description: Returns the preserve_trailing_whitespace flag.  See
//               set_preserve_trailing_whitespace().
////////////////////////////////////////////////////////////////////
INLINE bool TextProperties::
get_preserve_trailing_whitespace() const {
  return _preserve_trailing_whitespace;
}

////////////////////////////////////////////////////////////////////
//     Function: TextProperties::set_text_color
//       Access: Published
//  Description:
////////////////////////////////////////////////////////////////////
INLINE void TextProperties::
set_text_color(float r, float g, float b, float a) {
  set_text_color(Colorf(r, g, b, a));
}

////////////////////////////////////////////////////////////////////
//     Function: TextProperties::set_text_color
//       Access: Published
//  Description:
////////////////////////////////////////////////////////////////////
INLINE void TextProperties::
set_text_color(const Colorf &text_color) {
  _text_color = text_color;
  _specified |= F_has_text_color;
}

////////////////////////////////////////////////////////////////////
//     Function: TextProperties::clear_text_color
//       Access: Published
//  Description: Removes the text color specification; the text will
//               be colored whatever it was in the source font file.
////////////////////////////////////////////////////////////////////
INLINE void TextProperties::
clear_text_color() {
  _text_color.set(1.0f, 1.0f, 1.0f, 1.0f);
  _specified &= ~F_has_text_color;
}

////////////////////////////////////////////////////////////////////
//     Function: TextProperties::has_text_color
//       Access: Published
//  Description:
////////////////////////////////////////////////////////////////////
INLINE bool TextProperties::
has_text_color() const {
  return (_specified & F_has_text_color) != 0;
}

////////////////////////////////////////////////////////////////////
//     Function: TextProperties::get_text_color
//       Access: Published
//  Description:
////////////////////////////////////////////////////////////////////
INLINE Colorf TextProperties::
get_text_color() const {
  return _text_color;
}

////////////////////////////////////////////////////////////////////
//     Function: TextProperties::set_shadow_color
//       Access: Published
//  Description:
////////////////////////////////////////////////////////////////////
INLINE void TextProperties::
set_shadow_color(float r, float g, float b, float a) {
  set_shadow_color(Colorf(r, g, b, a));
}

////////////////////////////////////////////////////////////////////
//     Function: TextProperties::set_shadow_color
//       Access: Published
//  Description:
////////////////////////////////////////////////////////////////////
INLINE void TextProperties::
set_shadow_color(const Colorf &shadow_color) {
  _shadow_color = shadow_color;
  _specified |= F_has_shadow_color;
}

////////////////////////////////////////////////////////////////////
//     Function: TextProperties::clear_shadow_color
//       Access: Published
//  Description: Removes the shadow color specification.
////////////////////////////////////////////////////////////////////
INLINE void TextProperties::
clear_shadow_color() {
  _shadow_color.set(0.0f, 0.0f, 0.0f, 1.0f);
  _specified &= ~F_has_shadow_color;
}

////////////////////////////////////////////////////////////////////
//     Function: TextProperties::has_shadow_color
//       Access: Published
//  Description:
////////////////////////////////////////////////////////////////////
INLINE bool TextProperties::
has_shadow_color() const {
  return (_specified & F_has_shadow_color) != 0;
}

////////////////////////////////////////////////////////////////////
//     Function: TextProperties::get_shadow_color
//       Access: Published
//  Description:
////////////////////////////////////////////////////////////////////
INLINE Colorf TextProperties::
get_shadow_color() const {
  return _shadow_color;
}

////////////////////////////////////////////////////////////////////
//     Function: TextProperties::set_shadow
//       Access: Published
//  Description: Specifies that the text should be drawn with a
//               shadow, by creating a second copy of the text and
//               offsetting it slightly behind the first.
////////////////////////////////////////////////////////////////////
INLINE void TextProperties::
set_shadow(float xoffset, float yoffset) {
  set_shadow(LVecBase2f(xoffset, yoffset));
}

////////////////////////////////////////////////////////////////////
//     Function: TextProperties::set_shadow
//       Access: Published
//  Description: Specifies that the text should be drawn with a
//               shadow, by creating a second copy of the text and
//               offsetting it slightly behind the first.
////////////////////////////////////////////////////////////////////
INLINE void TextProperties::
set_shadow(const LVecBase2f &shadow_offset) {
  _shadow_offset = shadow_offset;
  _specified |= F_has_shadow;
}

////////////////////////////////////////////////////////////////////
//     Function: TextProperties::clear_shadow
//       Access: Published
//  Description: Specifies that a shadow will not be drawn behind the
//               text.
////////////////////////////////////////////////////////////////////
INLINE void TextProperties::
clear_shadow() {
  _specified &= ~F_has_shadow;
  _shadow_offset.set(0.0f, 0.0f);
}

////////////////////////////////////////////////////////////////////
//     Function: TextProperties::has_shadow
//       Access: Published
//  Description:
////////////////////////////////////////////////////////////////////
INLINE bool TextProperties::
has_shadow() const {
  return (_specified & F_has_shadow) != 0;
}

////////////////////////////////////////////////////////////////////
//     Function: TextProperties::get_shadow
//       Access: Published
//  Description: Returns the offset of the shadow as set by
//               set_shadow().  It is an error to call this if
//               has_shadow() is false.
////////////////////////////////////////////////////////////////////
INLINE LVector2f TextProperties::
get_shadow() const {
  return _shadow_offset;
}

////////////////////////////////////////////////////////////////////
//     Function: TextProperties::set_bin
//       Access: Published
//  Description: Names the CullBin that the text geometry should be
//               assigned to.  If this is set, then a CullBinAttrib
//               will be created to explicitly place each component in
//               the named bin.
//
//               The draw_order value will also be passed to each
//               CullBinAttrib as appropriate; this is particularly
//               useful if this names a CullBinFixed, e.g. "fixed".
////////////////////////////////////////////////////////////////////
INLINE void TextProperties::
set_bin(const string &bin) {
  _bin = bin;
  _specified |= F_has_bin;
}

////////////////////////////////////////////////////////////////////
//     Function: TextProperties::clear_bin
//       Access: Published
//  Description: Removes the effect of a previous call to
//               set_bin().  Text will be drawn in whatever bin
//               it would like to be drawn in, with no explicit
//               ordering.
////////////////////////////////////////////////////////////////////
INLINE void TextProperties::
clear_bin() {
  _bin = string();
  _specified &= ~F_has_bin;
}

////////////////////////////////////////////////////////////////////
//     Function: TextProperties::has_bin
//       Access: Published
//  Description: Returns true if an explicit drawing bin has been
//               set via set_bin(), false otherwise.
////////////////////////////////////////////////////////////////////
INLINE bool TextProperties::
has_bin() const {
  return (_specified & F_has_bin) != 0;
}

////////////////////////////////////////////////////////////////////
//     Function: TextProperties::get_bin
//       Access: Published
//  Description: Returns the drawing bin set with set_bin(), or empty
//               string if no bin has been set.
////////////////////////////////////////////////////////////////////
INLINE const string &TextProperties::
get_bin() const {
  return _bin;
}

////////////////////////////////////////////////////////////////////
//     Function: TextProperties::set_draw_order
//       Access: Published
//  Description: Sets the drawing order of text created by the
//               TextNode.  This is actually the draw order of the
//               card and frame.  The shadow is drawn at
//               _draw_order+1, and the text at _draw_order+2.
//
//               This affects the sorting order assigned to the nodes
//               as they are created, and also is passed to whatever
//               bin may be assigned via set_bin().
//
//               The return value is the first unused draw_order
//               number, e.g. _draw_order + 3.
////////////////////////////////////////////////////////////////////
INLINE int TextProperties::
set_draw_order(int draw_order) {
  _draw_order = draw_order;
  _specified |= F_has_draw_order;
  return _draw_order + 3;
}

////////////////////////////////////////////////////////////////////
//     Function: TextProperties::clear_draw_order
//       Access: Published
//  Description: 
////////////////////////////////////////////////////////////////////
INLINE void TextProperties::
clear_draw_order() {
  _draw_order = 1;
  _specified &= ~F_has_draw_order;
}

////////////////////////////////////////////////////////////////////
//     Function: TextProperties::has_draw_order
//       Access: Published
//  Description:
////////////////////////////////////////////////////////////////////
INLINE bool TextProperties::
has_draw_order() const {
  return (_specified & F_has_draw_order) != 0;
}

////////////////////////////////////////////////////////////////////
//     Function: TextProperties::get_draw_order
//       Access: Published
//  Description: Returns the drawing order set with set_draw_order().
////////////////////////////////////////////////////////////////////
INLINE int TextProperties::
get_draw_order() const {
  return _draw_order;
}

////////////////////////////////////////////////////////////////////
//     Function: TextProperties::set_tab_width
//       Access: Published
//  Description: Sets the width of each tab stop, in screen units.  A
//               tab character embedded in the text will advance the
//               horizontal position to the next tab stop.
////////////////////////////////////////////////////////////////////
INLINE void TextProperties::
set_tab_width(float tab_width) {
  _tab_width = tab_width;
  _specified |= F_has_tab_width;
}

////////////////////////////////////////////////////////////////////
//     Function: TextProperties::clear_tab_width
//       Access: Published
//  Description: 
////////////////////////////////////////////////////////////////////
INLINE void TextProperties::
clear_tab_width() {
  _tab_width = text_tab_width;
  _specified &= ~F_has_tab_width;
}

////////////////////////////////////////////////////////////////////
//     Function: TextProperties::has_tab_width
//       Access: Published
//  Description:
////////////////////////////////////////////////////////////////////
INLINE bool TextProperties::
has_tab_width() const {
  return (_specified & F_has_tab_width) != 0;
}

////////////////////////////////////////////////////////////////////
//     Function: TextProperties::get_tab_width
//       Access: Published
//  Description: Returns the width set via set_tab_width().
////////////////////////////////////////////////////////////////////
INLINE float TextProperties::
get_tab_width() const {
  return _tab_width;
}

////////////////////////////////////////////////////////////////////
//     Function: TextProperties::set_glyph_scale
//       Access: Published
//  Description: Specifies the factor by which to scale each letter of
//               the text as it is placed.  This can be used (possibly
//               in conjunction with set_glyph_shift()) to implement
//               superscripting or subscripting.
////////////////////////////////////////////////////////////////////
INLINE void TextProperties::
set_glyph_scale(float glyph_scale) {
  _glyph_scale = glyph_scale;
  _specified |= F_has_glyph_scale;
}

////////////////////////////////////////////////////////////////////
//     Function: TextProperties::clear_glyph_scale
//       Access: Published
//  Description: 
////////////////////////////////////////////////////////////////////
INLINE void TextProperties::
clear_glyph_scale() {
  _specified &= ~F_has_glyph_scale;
  _glyph_scale = 0.0f;
}

////////////////////////////////////////////////////////////////////
//     Function: TextProperties::has_glyph_scale
//       Access: Published
//  Description:
////////////////////////////////////////////////////////////////////
INLINE bool TextProperties::
has_glyph_scale() const {
  return (_specified & F_has_glyph_scale) != 0;
}

////////////////////////////////////////////////////////////////////
//     Function: TextProperties::get_glyph_scale
//       Access: Published
//  Description: Returns the scale factor of each letter as specified
//               by set_glyph_scale().
////////////////////////////////////////////////////////////////////
INLINE float TextProperties::
get_glyph_scale() const {
  return _glyph_scale;
}

////////////////////////////////////////////////////////////////////
//     Function: TextProperties::set_glyph_shift
//       Access: Published
//  Description: Specifies a vertical amount to shift each letter of
//               the text as it is placed.  This can be used (possibly
//               in conjunction with set_glyph_scale()) to implement
//               superscripting or subscripting.
////////////////////////////////////////////////////////////////////
INLINE void TextProperties::
set_glyph_shift(float glyph_shift) {
  _glyph_shift = glyph_shift;
  _specified |= F_has_glyph_shift;
}

////////////////////////////////////////////////////////////////////
//     Function: TextProperties::clear_glyph_shift
//       Access: Published
//  Description: 
////////////////////////////////////////////////////////////////////
INLINE void TextProperties::
clear_glyph_shift() {
  _specified &= ~F_has_glyph_shift;
  _glyph_shift = 0.0f;
}

////////////////////////////////////////////////////////////////////
//     Function: TextProperties::has_glyph_shift
//       Access: Published
//  Description:
////////////////////////////////////////////////////////////////////
INLINE bool TextProperties::
has_glyph_shift() const {
  return (_specified & F_has_glyph_shift) != 0;
}

////////////////////////////////////////////////////////////////////
//     Function: TextProperties::get_glyph_shift
//       Access: Published
//  Description: Returns the vertical shift of each letter as
//               specified by set_glyph_shift().
////////////////////////////////////////////////////////////////////
INLINE float TextProperties::
get_glyph_shift() const {
  return _glyph_shift;
}
