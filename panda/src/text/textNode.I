// Filename: textNode.I
// Created by:  drose (13Mar02)
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
//     Function: TextNode::freeze
//       Access: Published
//  Description: Freezes the TextNode in its current state, so that
//               updates will not immediately be displayed.  A series
//               of state changes may then be applied in succession,
//               which will not force the TextNode to be recomputed.
//               When thaw() is later called, the TextNode will update
//               itself exactly once to reflect all the state changes
//               that were made.
//
//               freeze() and thaw() can nest.  Strictly speaking,
//               each call to freeze() increments the current freeze
//               level, while each call to thaw() decrements it.  The
//               TextNode will only be updated when the current freeze
//               level is zero.
//
//               The return value of freeze() is the freeze level
//               *before* the freeze took place.  This number should
//               match the return value of the matching thaw().
////////////////////////////////////////////////////////////////////
INLINE int TextNode::
freeze() {
  if (text_cat.is_debug()) {
    text_cat.debug()
      << "Freezing " << this->get_name() << ", level = "
      << _freeze_level << "\n";
  }
  return _freeze_level++;
}

////////////////////////////////////////////////////////////////////
//     Function: TextNode::get_freeze_level
//       Access: Published
//  Description: Returns the current freeze level.  The TextNode will
//               not be updated visually unless this number is zero.
//               See freeze().
////////////////////////////////////////////////////////////////////
INLINE int TextNode::
get_freeze_level() const {
  return _freeze_level;
}

////////////////////////////////////////////////////////////////////
//     Function: TextNode::thaw
//       Access: Published
//  Description: Allows changes made since the last freeze() to be
//               visible.  Strictly speaking, this actually decrements
//               the freeze level, and updates the TextNode if the
//               level reaches zero.  The return value is the new
//               freeze level after adjusting.  See freeze().
////////////////////////////////////////////////////////////////////
INLINE int TextNode::
thaw() {
  if (text_cat.is_debug()) {
    text_cat.debug()
      << "Thawing " << this->get_name() << ", level = "
      << _freeze_level-1 << "\n";
  }
  nassertr(_freeze_level > 0, _freeze_level);
  _freeze_level--;

  if (_freeze_level == 0 && _needs_rebuild) {
    do_rebuild();
  }

  return _freeze_level;
}

////////////////////////////////////////////////////////////////////
//     Function: TextNode::set_font
//       Access: Published
//  Description: Sets the font that will be used when making text.
////////////////////////////////////////////////////////////////////
INLINE void TextNode::
set_font(TextFont *font) {
  if (_font != font) {
    _font = font;
    rebuild(true);
  }
}

////////////////////////////////////////////////////////////////////
//     Function: TextNode::get_font
//       Access: Published
//  Description: Returns the font currently in use.
////////////////////////////////////////////////////////////////////
INLINE TextFont *TextNode::
get_font() const {
  return _font;
}

////////////////////////////////////////////////////////////////////
//     Function: TextNode::set_encoding
//       Access: Published
//  Description: Specifies how the string set via set_text() is to be
//               interpreted.  The default, E_iso8859, means a
//               standard string with one-byte characters
//               (i.e. ASCII).  Other encodings are possible to take
//               advantage of character sets with more than 256
//               characters.
//
//               This affects only future calls to set_text(); it does
//               not change text that was set previously.
////////////////////////////////////////////////////////////////////
INLINE void TextNode::
set_encoding(TextNode::Encoding encoding) {
  // Force the previously-set strings to be encoded or decoded now.
  get_text();
  get_wtext();
  _encoding = encoding;
}

////////////////////////////////////////////////////////////////////
//     Function: TextNode::get_encoding
//       Access: Published
//  Description: Returns the encoding by which the string set via
//               set_text() is to be interpreted.  See set_encoding().
////////////////////////////////////////////////////////////////////
INLINE TextNode::Encoding TextNode::
get_encoding() const {
  return _encoding;
}

////////////////////////////////////////////////////////////////////
//     Function: TextNode::set_expand_amp
//       Access: Published
//  Description: Sets the state of the expand_amp flag.  When this is
//               true, embedded ampersands in the text string are
//               expanded to special characters according to a subset
//               of the HTML conventions.  When this is false,
//               ampersands are treated as ordinary characters.
//
//               This affects only future calls to set_text(); it does
//               not change text that was set previously.
////////////////////////////////////////////////////////////////////
INLINE void TextNode::
set_expand_amp(bool expand_amp) {
  if (expand_amp) {
    _flags |= F_expand_amp;
  } else {
    _flags &= ~F_expand_amp;
  }
}

////////////////////////////////////////////////////////////////////
//     Function: TextNode::get_expand_amp
//       Access: Published
//  Description: Returns the state of the expand_amp flag.  See
//               set_expand_amp().
////////////////////////////////////////////////////////////////////
INLINE bool TextNode::
get_expand_amp() const {
  return (_flags & F_expand_amp) != 0;
}

////////////////////////////////////////////////////////////////////
//     Function: TextNode::get_line_height
//       Access: Published
//  Description: Returns the number of units high each line of text
//               is.  This is based on the font.
////////////////////////////////////////////////////////////////////
INLINE float TextNode::
get_line_height() const {
  return (_font == (TextFont *)NULL) ? 0.0 : _font->get_line_height();
}

////////////////////////////////////////////////////////////////////
//     Function: TextNode::set_slant
//       Access: Published
//  Description:
////////////////////////////////////////////////////////////////////
INLINE void TextNode::
set_slant(float slant) {
  if (_slant != slant) {
    _slant = slant;
    rebuild(true);
  }
}

////////////////////////////////////////////////////////////////////
//     Function: TextNode::get_slant
//       Access: Published
//  Description:
////////////////////////////////////////////////////////////////////
INLINE float TextNode::
get_slant() const {
  return _slant;
}

////////////////////////////////////////////////////////////////////
//     Function: TextNode::set_align
//       Access: Published
//  Description:
////////////////////////////////////////////////////////////////////
INLINE void TextNode::
set_align(TextNode::Alignment align_type) {
  if (_align != align_type) {
    _align = align_type;
    rebuild(true);
  }
}

////////////////////////////////////////////////////////////////////
//     Function: TextNode::get_align
//       Access: Published
//  Description:
////////////////////////////////////////////////////////////////////
INLINE TextNode::Alignment TextNode::
get_align() const {
  return _align;
}

////////////////////////////////////////////////////////////////////
//     Function: TextNode::set_wordwrap
//       Access: Published
//  Description: Sets the TextNode up to automatically wordwrap text
//               that exceeds the indicated width.
////////////////////////////////////////////////////////////////////
INLINE void TextNode::
set_wordwrap(float wordwrap) {
  if (!has_wordwrap() || _wordwrap_width != wordwrap) {
    _flags |= F_has_wordwrap;
    _wordwrap_width = wordwrap;
    rebuild(true);
  }
}

////////////////////////////////////////////////////////////////////
//     Function: TextNode::clear_wordwrap
//       Access: Published
//  Description: Removes the wordwrap setting from the TextNode.  Text
//               will be as wide as it is.
////////////////////////////////////////////////////////////////////
INLINE void TextNode::
clear_wordwrap() {
  if (has_wordwrap()) {
    _flags &= ~F_has_wordwrap;
    rebuild(true);
  }
}

////////////////////////////////////////////////////////////////////
//     Function: TextNode::has_wordwrap
//       Access: Published
//  Description:
////////////////////////////////////////////////////////////////////
INLINE bool TextNode::
has_wordwrap() const {
  return (_flags & F_has_wordwrap) != 0;
}

////////////////////////////////////////////////////////////////////
//     Function: TextNode::get_wordwrap
//       Access: Published
//  Description:
////////////////////////////////////////////////////////////////////
INLINE float TextNode::
get_wordwrap() const {
  return _wordwrap_width;
}

////////////////////////////////////////////////////////////////////
//     Function: TextNode::set_text_color
//       Access: Published
//  Description:
////////////////////////////////////////////////////////////////////
INLINE void TextNode::
set_text_color(float r, float g, float b, float a) {
  set_text_color(Colorf(r, g, b, a));
}

////////////////////////////////////////////////////////////////////
//     Function: TextNode::set_text_color
//       Access: Published
//  Description:
////////////////////////////////////////////////////////////////////
INLINE void TextNode::
set_text_color(const Colorf &text_color) {
  if (!has_text_color() || _text_color != text_color) {
    _text_color = text_color;
    _flags |= F_has_text_color;
    rebuild(false);
  }
}

////////////////////////////////////////////////////////////////////
//     Function: TextNode::clear_text_color
//       Access: Published
//  Description: Removes the text color specification; the text will
//               be colored whatever it was in the source font file.
////////////////////////////////////////////////////////////////////
INLINE void TextNode::
clear_text_color() {
  if (has_text_color()) {
    _flags &= ~F_has_text_color;
    rebuild(false);
  }
}

////////////////////////////////////////////////////////////////////
//     Function: TextNode::has_text_color
//       Access: Published
//  Description:
////////////////////////////////////////////////////////////////////
INLINE bool TextNode::
has_text_color() const {
  return (_flags & F_has_text_color) != 0;
}

////////////////////////////////////////////////////////////////////
//     Function: TextNode::get_text_color
//       Access: Published
//  Description:
////////////////////////////////////////////////////////////////////
INLINE Colorf TextNode::
get_text_color() const {
  return _text_color;
}

////////////////////////////////////////////////////////////////////
//     Function: TextNode::set_frame_color
//       Access: Published
//  Description:
////////////////////////////////////////////////////////////////////
INLINE void TextNode::
set_frame_color(float r, float g, float b, float a) {
  set_frame_color(Colorf(r, g, b, a));
}

////////////////////////////////////////////////////////////////////
//     Function: TextNode::set_frame_color
//       Access: Published
//  Description:
////////////////////////////////////////////////////////////////////
INLINE void TextNode::
set_frame_color(const Colorf &frame_color) {
  if (_frame_color != frame_color) {
    _frame_color = frame_color;
    rebuild(false);
  }
}

////////////////////////////////////////////////////////////////////
//     Function: TextNode::get_frame_color
//       Access: Published
//  Description:
////////////////////////////////////////////////////////////////////
INLINE Colorf TextNode::
get_frame_color() const {
  return _frame_color;
}

////////////////////////////////////////////////////////////////////
//     Function: TextNode::set_card_border
//       Access: Published
//  Description:
////////////////////////////////////////////////////////////////////
INLINE void TextNode::
set_card_border(float size, float uv_portion) {
  if (!has_card_border() || _card_border_size != size || _card_border_uv_portion != uv_portion) {
    _flags |= F_has_card_border;
    _card_border_size = size;
    _card_border_uv_portion = uv_portion;
    rebuild(false);
  }
}

////////////////////////////////////////////////////////////////////
//     Function: TextNode::clear_card_border
//       Access: Published
//  Description:
////////////////////////////////////////////////////////////////////
INLINE void TextNode::
clear_card_border() {
  if (has_card_border()) {
    _flags &= ~F_has_card_border;
    rebuild(false);
  }
}

////////////////////////////////////////////////////////////////////
//     Function: TextNode::get_card_border_size
//       Access: Published
//  Description:
////////////////////////////////////////////////////////////////////
INLINE float TextNode::
get_card_border_size() const {
  return _card_border_size;
}

////////////////////////////////////////////////////////////////////
//     Function: TextNode::get_card_border_uv_portion
//       Access: Published
//  Description:
////////////////////////////////////////////////////////////////////
INLINE float TextNode::
get_card_border_uv_portion() const {
  return _card_border_uv_portion;
}

////////////////////////////////////////////////////////////////////
//     Function: TextNode::has_card_border
//       Access: Published
//  Description:
////////////////////////////////////////////////////////////////////
INLINE bool TextNode::
has_card_border() const {
  return (_flags & F_has_card_border) != 0;
}

////////////////////////////////////////////////////////////////////
//     Function: TextNode::set_card_color
//       Access: Published
//  Description:
////////////////////////////////////////////////////////////////////
INLINE void TextNode::
set_card_color(float r, float g, float b, float a) {
  set_card_color(Colorf(r, g, b, a));
}

////////////////////////////////////////////////////////////////////
//     Function: TextNode::set_card_color
//       Access: Published
//  Description:
////////////////////////////////////////////////////////////////////
INLINE void TextNode::
set_card_color(const Colorf &card_color) {
  if (_card_color != card_color) {
    _card_color = card_color;
    rebuild(false);
  }
}

////////////////////////////////////////////////////////////////////
//     Function: TextNode::get_card_color
//       Access: Published
//  Description:
////////////////////////////////////////////////////////////////////
INLINE Colorf TextNode::
get_card_color() const {
  return _card_color;
}

////////////////////////////////////////////////////////////////////
//     Function: TextNode::set_card_texture
//       Access: Published
//  Description:
////////////////////////////////////////////////////////////////////
INLINE void TextNode::
set_card_texture(Texture *card_texture) {
  if (card_texture == (Texture *)NULL) {
    clear_card_texture();
  } else {
    if (!has_card_texture() || _card_texture != card_texture) {
      _flags |= F_has_card_texture;
      _card_texture = card_texture;
      rebuild(false);
    }
  }
}

////////////////////////////////////////////////////////////////////
//     Function: TextNode::clear_card_texture
//       Access: Published
//  Description:
////////////////////////////////////////////////////////////////////
INLINE void TextNode::
clear_card_texture() {
  if (has_card_texture()) {
    _flags &= ~F_has_card_texture;
    _card_texture = NULL;
    rebuild(false);
  }
}

////////////////////////////////////////////////////////////////////
//     Function: TextNode::has_card_texture
//       Access: Published
//  Description:
////////////////////////////////////////////////////////////////////
INLINE bool TextNode::
has_card_texture() const {
  return (_flags & F_has_card_texture) != 0;
}

////////////////////////////////////////////////////////////////////
//     Function: TextNode::get_card_texture
//       Access: Published
//  Description:
////////////////////////////////////////////////////////////////////
INLINE Texture *TextNode::
get_card_texture() const {
  return _card_texture;
}

////////////////////////////////////////////////////////////////////
//     Function: TextNode::set_shadow_color
//       Access: Published
//  Description:
////////////////////////////////////////////////////////////////////
INLINE void TextNode::
set_shadow_color(float r, float g, float b, float a) {
  set_shadow_color(Colorf(r, g, b, a));
}

////////////////////////////////////////////////////////////////////
//     Function: TextNode::set_shadow_color
//       Access: Published
//  Description:
////////////////////////////////////////////////////////////////////
INLINE void TextNode::
set_shadow_color(const Colorf &shadow_color) {
  if (_shadow_color != shadow_color) {
    _shadow_color = shadow_color;
    rebuild(false);
  }
}

////////////////////////////////////////////////////////////////////
//     Function: TextNode::get_shadow_color
//       Access: Published
//  Description:
////////////////////////////////////////////////////////////////////
INLINE Colorf TextNode::
get_shadow_color() const {
  return _shadow_color;
}

////////////////////////////////////////////////////////////////////
//     Function: TextNode::set_frame_as_margin
//       Access: Published
//  Description: Specifies that a border will be drawn around the text
//               when it is next created.  The parameters are the
//               amount of additional padding to insert between the
//               frame and the text in each dimension, and all should
//               generally be positive.
////////////////////////////////////////////////////////////////////
INLINE void TextNode::
set_frame_as_margin(float left, float right, float bottom, float top) {
  _flags |= (F_has_frame | F_frame_as_margin);
  _frame_ul.set(left, top);
  _frame_lr.set(right, bottom);
  rebuild(false);
}

////////////////////////////////////////////////////////////////////
//     Function: TextNode::set_frame_actual
//       Access: Published
//  Description: Similar to set_frame_as_margin, except the frame is
//               specified in actual coordinate units (relative to
//               the text's origin), irrespective of the size of the
//               text.  The left and bottom coordinates should
//               generally be negative, while the right and top
//               coordinates should generally be positive.
////////////////////////////////////////////////////////////////////
INLINE void TextNode::
set_frame_actual(float left, float right, float bottom, float top) {
  _flags |= F_has_frame;
  _flags &= ~F_frame_as_margin;
  _frame_ul.set(left, top);
  _frame_lr.set(right, bottom);
  rebuild(false);
}

////////////////////////////////////////////////////////////////////
//     Function: TextNode::clear_frame
//       Access: Published
//  Description: Specifies that a border will not be drawn around the
//               text.
////////////////////////////////////////////////////////////////////
INLINE void TextNode::
clear_frame() {
  _flags &= ~F_has_frame;
  rebuild(false);
}

////////////////////////////////////////////////////////////////////
//     Function: TextNode::has_frame
//       Access: Published
//  Description:
////////////////////////////////////////////////////////////////////
INLINE bool TextNode::
has_frame() const {
  return (_flags & F_has_frame) != 0;
}

////////////////////////////////////////////////////////////////////
//     Function: TextNode::is_frame_as_margin
//       Access: Published
//  Description: If this is true, the frame was set via a call to
//               set_frame_as_margin(), and the dimension of the frame
//               as returned by get_frame_as_set() represent a margin
//               all around the text.  If false, then the frame was
//               set via a call to set_frame_actual(), and the
//               dimensions of the frame as returned by
//               get_frame_as_set() are relative to the text's origin.
////////////////////////////////////////////////////////////////////
INLINE bool TextNode::
is_frame_as_margin() const {
  nassertr(has_frame(), false);
  return (_flags & F_frame_as_margin) != 0;
}

////////////////////////////////////////////////////////////////////
//     Function: TextNode::get_frame_as_set
//       Access: Published
//  Description: Returns the dimensions of the frame as set by
//               set_frame_as_margin() or set_frame_actual().  Use
//               is_frame_actual() to determine how to interpret the
//               values returned by this function.  It is an error to
//               call this if has_frame() is false.
////////////////////////////////////////////////////////////////////
INLINE LVecBase4f TextNode::
get_frame_as_set() const {
  nassertr(has_frame(), LVecBase4f(0.0, 0.0, 0.0, 0.0));
  return LVecBase4f(_frame_ul[0], _frame_lr[0], _frame_lr[1], _frame_ul[1]);
}

////////////////////////////////////////////////////////////////////
//     Function: TextNode::get_frame_actual
//       Access: Published
//  Description: Returns the actual dimensions of the frame around the
//               text.  If the frame was set via set_frame_as_margin(),
//               the result returned by this function reflects the
//               size of the current text; if the frame was set via
//               set_frame_actual(), this returns the values
//               actually set.
////////////////////////////////////////////////////////////////////
INLINE LVecBase4f TextNode::
get_frame_actual() const {
  nassertr(has_frame(), LVecBase4f(0.0, 0.0, 0.0, 0.0));
  if (is_frame_as_margin()) {
    return LVecBase4f(get_left() - _frame_ul[0],
                      get_right() + _frame_lr[0],
                      get_bottom() - _frame_lr[1],
                      get_top() + _frame_ul[1]);
  } else {
    return get_frame_as_set();
  }
}

////////////////////////////////////////////////////////////////////
//     Function: TextNode::set_frame_line_width
//       Access: Published
//  Description: Specifies the thickness of the lines that will be
//               used to draw the frame.
////////////////////////////////////////////////////////////////////
INLINE void TextNode::
set_frame_line_width(float frame_width) {
  _frame_width = frame_width;
}

////////////////////////////////////////////////////////////////////
//     Function: TextNode::get_frame_line_width
//       Access: Published
//  Description: Returns the thickness of the lines that will be
//               used to draw the frame.
////////////////////////////////////////////////////////////////////
INLINE float TextNode::
get_frame_line_width() const {
  return _frame_width;
}

////////////////////////////////////////////////////////////////////
//     Function: TextNode::set_frame_corners
//       Access: Published
//  Description: Enables or disables the drawing of corners for the
//               frame.  These are extra points drawn at each of the
//               four corners, to soften the ugly edges generated when
//               the line width is greater than one.
////////////////////////////////////////////////////////////////////
INLINE void TextNode::
set_frame_corners(bool corners) {
  if (corners) {
    _flags |= F_frame_corners;
  } else {
    _flags &= ~F_frame_corners;
  }
}

////////////////////////////////////////////////////////////////////
//     Function: TextNode::get_frame_corners
//       Access: Published
//  Description:
////////////////////////////////////////////////////////////////////
INLINE bool TextNode::
get_frame_corners() const {
  return (_flags & F_frame_corners) != 0;
}

////////////////////////////////////////////////////////////////////
//     Function: TextNode::set_card_as_margin
//       Access: Published
//  Description: Specifies that a (possibly opaque or semitransparent)
//               card will be held behind the text when it is next
//               created.  Like set_frame_as_margin, the parameters are
//               the amount of additional padding to insert around the
//               text in each dimension, and all should generally be
//               positive.
////////////////////////////////////////////////////////////////////
INLINE void TextNode::
set_card_as_margin(float left, float right, float bottom, float top) {
  _flags |= (F_has_card | F_card_as_margin);
  _card_ul.set(left, top);
  _card_lr.set(right, bottom);
  rebuild(false);
}

////////////////////////////////////////////////////////////////////
//     Function: TextNode::set_card_actual
//       Access: Published
//  Description: Similar to set_card_as_margin, except the card is
//               specified in actual coordinate units (relative to
//               the text's origin), irrespective of the size of the
//               text.  The left and bottom coordinates should
//               generally be negative, while the right and top
//               coordinates should generally be positive.
////////////////////////////////////////////////////////////////////
INLINE void TextNode::
set_card_actual(float left, float right, float bottom, float top) {
  _flags |= F_has_card;
  _flags &= ~F_card_as_margin;
  _card_ul.set(left, top);
  _card_lr.set(right, bottom);
  rebuild(false);
}

////////////////////////////////////////////////////////////////////
//     Function: TextNode::clear_card
//       Access: Published
//  Description: Specifies that a card will not be drawn behind the
//               text.
////////////////////////////////////////////////////////////////////
INLINE void TextNode::
clear_card() {
  _flags &= ~F_has_card;
  rebuild(false);
}

////////////////////////////////////////////////////////////////////
//     Function: TextNode::has_card
//       Access: Published
//  Description:
////////////////////////////////////////////////////////////////////
INLINE bool TextNode::
has_card() const {
  return (_flags & F_has_card) != 0;
}

////////////////////////////////////////////////////////////////////
//     Function: TextNode::is_card_as_margin
//       Access: Published
//  Description: If this is true, the card was set via a call to
//               set_card_as_margin(), and the dimension of the card
//               as returned by get_card_as_set() represent a margin
//               all around the text.  If false, then the card was
//               set via a call to set_card_actual(), and the
//               dimensions of the card as returned by
//               get_card_as_set() are relative to the text's origin.
////////////////////////////////////////////////////////////////////
INLINE bool TextNode::
is_card_as_margin() const {
  nassertr(has_card(), false);
  return (_flags & F_card_as_margin) != 0;
}

////////////////////////////////////////////////////////////////////
//     Function: TextNode::get_card_as_set
//       Access: Published
//  Description: Returns the dimensions of the card as set by
//               set_card_as_margin() or set_card_actual().  Use
//               is_card_actual() to determine how to interpret the
//               values returned by this function.  It is an error to
//               call this if has_card() is false.
////////////////////////////////////////////////////////////////////
INLINE LVecBase4f TextNode::
get_card_as_set() const {
  nassertr(has_card(), LVecBase4f(0.0, 0.0, 0.0, 0.0));
  return LVecBase4f(_card_ul[0], _card_lr[0], _card_lr[1], _card_ul[1]);
}

////////////////////////////////////////////////////////////////////
//     Function: TextNode::get_card_actual
//       Access: Published
//  Description: Returns the actual dimensions of the card around the
//               text.  If the card was set via set_card_as_margin(),
//               the result returned by this function reflects the
//               size of the current text; if the card was set via
//               set_card_actual(), this returns the values
//               actually set.
//
//               If the text has no card at all, this returns the
//               dimensions of the text itself, as if the card were
//               set with a margin of 0, 0, 0, 0.
////////////////////////////////////////////////////////////////////
INLINE LVecBase4f TextNode::
get_card_actual() const {
  if (!has_card()) {
    return LVecBase4f(get_left(), get_right(), get_bottom(), get_top());

  } else if (is_card_as_margin()) {
    return LVecBase4f(get_left() - _card_ul[0],
                      get_right() + _card_lr[0],
                      get_bottom() - _card_lr[1],
                      get_top() + _card_ul[1]);
  } else {
    return get_card_as_set();
  }
}

////////////////////////////////////////////////////////////////////
//     Function: TextNode::get_card_transformed
//       Access: Published
//  Description: Returns the actual card dimensions, transformed by
//               the matrix set by set_transform().  This returns the
//               card dimensions in actual coordinates as seen by the
//               rest of the world.  Also see get_upper_left_3d() and
//               get_lower_right_3d().
////////////////////////////////////////////////////////////////////
INLINE LVecBase4f TextNode::
get_card_transformed() const {
  LVecBase4f card = get_card_actual();
  LPoint3f ul = LPoint3f(card[0], 0.0, card[3]) * _transform;
  LPoint3f lr = LPoint3f(card[1], 0.0, card[2]) * _transform;

  return LVecBase4f(ul[0], lr[0], lr[2], ul[2]);
}

////////////////////////////////////////////////////////////////////
//     Function: TextNode::set_shadow
//       Access: Published
//  Description: Specifies that the text should be drawn with a
//               shadow, by creating a second copy of the text and
//               offsetting it slightly behind the first.
////////////////////////////////////////////////////////////////////
INLINE void TextNode::
set_shadow(float xoffset, float yoffset) {
  _flags |= F_has_shadow;
  _shadow_offset.set(xoffset, yoffset);
  rebuild(false);
}

////////////////////////////////////////////////////////////////////
//     Function: TextNode::clear_shadow
//       Access: Published
//  Description: Specifies that a shadow will not be drawn behind the
//               text.
////////////////////////////////////////////////////////////////////
INLINE void TextNode::
clear_shadow() {
  _flags &= ~F_has_shadow;
  rebuild(false);
}

////////////////////////////////////////////////////////////////////
//     Function: TextNode::has_shadow
//       Access: Published
//  Description:
////////////////////////////////////////////////////////////////////
INLINE bool TextNode::
has_shadow() const {
  return (_flags & F_has_shadow) != 0;
}

////////////////////////////////////////////////////////////////////
//     Function: TextNode::get_shadow
//       Access: Published
//  Description: Returns the offset of the shadow as set by
//               set_shadow().  It is an error to call this if
//               has_shadow() is false.
////////////////////////////////////////////////////////////////////
INLINE LVecBase2f TextNode::
get_shadow() const {
  nassertr(has_shadow(), LVecBase2f(0.0, 0.0));
  return _shadow_offset;
}

////////////////////////////////////////////////////////////////////
//     Function: TextNode::set_bin
//       Access: Published
//  Description: Names the GeomBin that the TextNode geometry should
//               be assigned to.  If this is set, then a
//               GeomBinTransition will be created to explicitly place
//               each component in the named bin.
//
//               The draw_order value will also be passed to each
//               GeomBinTransition as appropriate; this is
//               particularly useful if this names a GeomBinFixed,
//               e.g. "fixed".
////////////////////////////////////////////////////////////////////
INLINE void TextNode::
set_bin(const string &bin) {
  _bin = bin;
  rebuild(false);
}

////////////////////////////////////////////////////////////////////
//     Function: TextNode::clear_bin
//       Access: Published
//  Description: Removes the effect of a previous call to
//               set_bin().  Text will be drawn in whatever bin
//               it would like to be drawn in, with no explicit
//               ordering.
////////////////////////////////////////////////////////////////////
INLINE void TextNode::
clear_bin() {
  _bin = string();
}

////////////////////////////////////////////////////////////////////
//     Function: TextNode::has_bin
//       Access: Published
//  Description: Returns true if an explicit drawing bin has been
//               set via set_bin(), false otherwise.
////////////////////////////////////////////////////////////////////
INLINE bool TextNode::
has_bin() const {
  return !_bin.empty();
}

////////////////////////////////////////////////////////////////////
//     Function: TextNode::get_bin
//       Access: Published
//  Description: Returns the drawing bin set with set_bin(), or empty
//               string if no bin has been set.
////////////////////////////////////////////////////////////////////
INLINE const string &TextNode::
get_bin() const {
  return _bin;
}

////////////////////////////////////////////////////////////////////
//     Function: TextNode::set_draw_order
//       Access: Published
//  Description: Sets the drawing order of text created by the
//               TextMaker.  This is actually the draw order of the
//               card and frame.  The shadow is drawn at
//               _draw_order+1, and the text at _draw_order+2.
//
//               This affects the sorting order assigned to the arcs
//               as they are created, and also is passed to whatever
//               bin may be assigned via set_bin().
//
//               The return value is the first unused draw_order
//               number, e.g. _draw_order + 3.
////////////////////////////////////////////////////////////////////
INLINE int TextNode::
set_draw_order(int draw_order) {
  _draw_order = draw_order;
  rebuild(false);
  return _draw_order + 3;
}

////////////////////////////////////////////////////////////////////
//     Function: TextNode::get_draw_order
//       Access: Published
//  Description: Returns the drawing order set with set_draw_order().
////////////////////////////////////////////////////////////////////
INLINE int TextNode::
get_draw_order() const {
  return _draw_order;
}

////////////////////////////////////////////////////////////////////
//     Function: TextNode::set_transform
//       Access: Published
//  Description: Sets an additional transform that is applied to the
//               entire text paragraph.
////////////////////////////////////////////////////////////////////
INLINE void TextNode::
set_transform(const LMatrix4f &transform) {
  _transform = transform;
  rebuild(true);
}

////////////////////////////////////////////////////////////////////
//     Function: TextNode::get_transform
//       Access: Published
//  Description:
////////////////////////////////////////////////////////////////////
INLINE LMatrix4f TextNode::
get_transform() const {
  return _transform;
}

////////////////////////////////////////////////////////////////////
//     Function: TextNode::set_coordinate_system
//       Access: Published
//  Description: Specifies the coordinate system in which the text
//               will be generated.
////////////////////////////////////////////////////////////////////
INLINE void TextNode::
set_coordinate_system(CoordinateSystem coordinate_system) {
  _coordinate_system = coordinate_system;
  rebuild(true);
}

////////////////////////////////////////////////////////////////////
//     Function: TextNode::get_coordinate_system
//       Access: Published
//  Description:
////////////////////////////////////////////////////////////////////
INLINE CoordinateSystem TextNode::
get_coordinate_system() const {
  return _coordinate_system;
}

////////////////////////////////////////////////////////////////////
//     Function: TextNode::set_text
//       Access: Published
//  Description: Changes the text that is displayed under the
//               TextNode.
////////////////////////////////////////////////////////////////////
INLINE void TextNode::
set_text(const string &text) {
  _text = text;
  _flags = (_flags | F_got_text) & ~F_got_wtext;
  rebuild(true);
}

////////////////////////////////////////////////////////////////////
//     Function: TextNode::clear_text
//       Access: Published
//  Description: Removes the text from the TextNode.
////////////////////////////////////////////////////////////////////
INLINE void TextNode::
clear_text() {
  _text = string();
  _wtext = wstring();
  _flags |= (F_got_text | F_got_wtext);
  rebuild(true);
}

////////////////////////////////////////////////////////////////////
//     Function: TextNode::has_text
//       Access: Published
//  Description:
////////////////////////////////////////////////////////////////////
INLINE bool TextNode::
has_text() const {
  if (_flags & F_got_wtext) {
    return !_wtext.empty();
  } else {
    return !_text.empty();
  }
}

////////////////////////////////////////////////////////////////////
//     Function: TextNode::get_text
//       Access: Published
//  Description:
////////////////////////////////////////////////////////////////////
INLINE string TextNode::
get_text() const {
  if ((_flags & F_got_text) == 0) {
    ((TextNode *)this)->_text = encode_wtext(_wtext);
    ((TextNode *)this)->_flags |= F_got_text;
  }
  return _text;
}

////////////////////////////////////////////////////////////////////
//     Function: TextNode::append_text
//       Access: Published
//  Description: Appends the indicates string to the end of the stored
//               text.
////////////////////////////////////////////////////////////////////
INLINE void TextNode::
append_text(const string &text) {
  _text = get_text() + text;
  _flags = (_flags | F_got_text) & ~F_got_wtext;
  rebuild(true);
}

////////////////////////////////////////////////////////////////////
//     Function: TextNode::append_char
//       Access: Published
//  Description: Appends a single character to the end of the stored
//               text.  This may be a wide character, up to 16 bits in
//               Unicode.
////////////////////////////////////////////////////////////////////
INLINE void TextNode::
append_char(int character) {
  _wtext = get_wtext() + wstring(1, (wchar_t)character);
  _flags = (_flags | F_got_wtext) & ~F_got_text;
  rebuild(true);
}

////////////////////////////////////////////////////////////////////
//     Function: TextNode::get_num_chars
//       Access: Published
//  Description: Returns the number of characters in the stored text.
//               This is a count of wide characters, after the string
//               has been decoded according to set_encoding().
////////////////////////////////////////////////////////////////////
INLINE int TextNode::
get_num_chars() const {
  return get_wtext().length();
}

////////////////////////////////////////////////////////////////////
//     Function: TextNode::get_char
//       Access: Published
//  Description: Returns the Unicode value of the nth character in the
//               stored text.  This may be a wide character (greater
//               than 255), after the string has been decoded
//               according to set_encoding().
////////////////////////////////////////////////////////////////////
INLINE int TextNode::
get_char(int index) const {
  get_wtext();
  nassertr(index >= 0 && index < (int)_wtext.length(), 0);
  return _wtext[index];
}

////////////////////////////////////////////////////////////////////
//     Function: TextNode::calc_width
//       Access: Published
//  Description: Returns the width of a single character of the font,
//               or 0.0 if the character is not known.  This may be a
//               wide character (greater than 255).
////////////////////////////////////////////////////////////////////
INLINE float TextNode::
calc_width(int character) const {
  nassertr(_font != (TextFont *)NULL, 0.0);
  return _font->calc_width(character);
}

////////////////////////////////////////////////////////////////////
//     Function: TextNode::calc_width
//       Access: Published
//  Description: Returns the width of a line of text of arbitrary
//               characters.  The line should not include the newline
//               character.
////////////////////////////////////////////////////////////////////
INLINE float TextNode::
calc_width(const string &line) const {
  nassertr(_font != (TextFont *)NULL, 0.0);
  return _font->calc_width(decode_text(line));
}

////////////////////////////////////////////////////////////////////
//     Function: TextNode::rebuild
//       Access: Published
//  Description: Updates the TextNode, if it is not frozen, or marks
//               the TextNode as requiring an update if it is.  If the
//               text is currently frozen, nothing will be done until
//               it is thawed, unless needs_measure is true, in which
//               case the text will be re-measured even if it is
//               currently frozen.
//
//               Normally, this function is called automatically
//               whenever any of the parameters changes.  It should
//               not need to be called explicitly unless something
//               goes wrong.
////////////////////////////////////////////////////////////////////
INLINE void TextNode::
rebuild(bool needs_measure) {
  if (_freeze_level <= 0) {
    do_rebuild();
  } else {
    _needs_rebuild = true;

    if (needs_measure) {
      measure();
    }
  }
}

////////////////////////////////////////////////////////////////////
//     Function: TextNode::measure
//       Access: Published
//  Description: Measures the extent of the text as it will be placed,
//               without actually placing it.  Normally, this function
//               is called automatically whenever any of the
//               parameters changes.  It should not need to be called
//               explicitly unless something goes wrong.
////////////////////////////////////////////////////////////////////
INLINE void TextNode::
measure() {
  do_measure();
}

////////////////////////////////////////////////////////////////////
//     Function: TextNode::get_left
//       Access: Published
//  Description: Returns the leftmost extent of the text in local 2-d
//               coordinates, unmodified by the set_transform()
//               matrix.
////////////////////////////////////////////////////////////////////
INLINE float TextNode::
get_left() const {
  return _ul2d[0];
}

////////////////////////////////////////////////////////////////////
//     Function: TextNode::get_right
//       Access: Published
//  Description: Returns the rightmost extent of the text in local 2-d
//               coordinates, unmodified by the set_transform()
//               matrix.
////////////////////////////////////////////////////////////////////
INLINE float TextNode::
get_right() const {
  return _lr2d[0];
}

////////////////////////////////////////////////////////////////////
//     Function: TextNode::get_bottom
//       Access: Published
//  Description: Returns the bottommost extent of the text in local
//               2-d coordinates, unmodified by the set_transform()
//               matrix.
////////////////////////////////////////////////////////////////////
INLINE float TextNode::
get_bottom() const {
  return _lr2d[1];
}

////////////////////////////////////////////////////////////////////
//     Function: TextNode::get_top
//       Access: Published
//  Description: Returns the topmost extent of the text in local 2-d
//               coordinates, unmodified by the set_transform()
//               matrix.
////////////////////////////////////////////////////////////////////
INLINE float TextNode::
get_top() const {
  return _ul2d[1];
}

////////////////////////////////////////////////////////////////////
//     Function: TextNode::get_height
//       Access: Published
//  Description: Returns the net height of the text in local 2-d
//               coordinates.
////////////////////////////////////////////////////////////////////
INLINE float TextNode::
get_height() const {
  return get_top() - get_bottom();
}

////////////////////////////////////////////////////////////////////
//     Function: TextNode::get_width
//       Access: Published
//  Description: Returns the net width of the text in local 2-d
//               coordinates.
////////////////////////////////////////////////////////////////////
INLINE float TextNode::
get_width() const {
  return get_right() - get_left();
}

////////////////////////////////////////////////////////////////////
//     Function: TextNode::get_upper_left_3d
//       Access: Published
//  Description: Returns the upper-left extent of the text object,
//               after it has been transformed into 3-d space by
//               applying the set_transform() matrix.
////////////////////////////////////////////////////////////////////
INLINE LPoint3f TextNode::
get_upper_left_3d() const {
  return _ul3d;
}

////////////////////////////////////////////////////////////////////
//     Function: TextNode::get_lower_right_3d
//       Access: Published
//  Description: Returns the lower-right extent of the text object,
//               after it has been transformed into 3-d space by
//               applying the set_transform() matrix.
////////////////////////////////////////////////////////////////////
INLINE LPoint3f TextNode::
get_lower_right_3d() const {
  return _lr3d;
}

////////////////////////////////////////////////////////////////////
//     Function: TextNode::get_num_rows
//       Access: Published
//  Description: Returns the number of rows of text that were
//               generated.  This counts word-wrapped rows as well as
//               rows generated due to embedded newlines.
////////////////////////////////////////////////////////////////////
INLINE int TextNode::
get_num_rows() const {
  return _num_rows;
}

////////////////////////////////////////////////////////////////////
//     Function: TextNode::set_wtext
//       Access: Public
//  Description: Changes the text that is displayed under the
//               TextNode, with a wide text.  This automatically sets
//               the string reported by get_text() to the 8-bit
//               encoded version of the same string.
////////////////////////////////////////////////////////////////////
INLINE void TextNode::
set_wtext(const wstring &wtext) {
  _wtext = wtext;
  _flags = (_flags | F_got_wtext) & ~F_got_text;
  rebuild(true);
}

////////////////////////////////////////////////////////////////////
//     Function: TextNode::get_wtext
//       Access: Public
//  Description: Returns the text associated with the TextNode, as a
//               wide-character string.
////////////////////////////////////////////////////////////////////
INLINE const wstring &TextNode::
get_wtext() const {
  if ((_flags & F_got_wtext) == 0) {
    ((TextNode *)this)->_wtext = decode_text(_text);
    ((TextNode *)this)->_flags |= F_got_wtext;
  }
  return _wtext;
}

////////////////////////////////////////////////////////////////////
//     Function: TextNode::append_wtext
//       Access: Public
//  Description: Appends the indicates string to the end of the stored
//               wide-character text.
////////////////////////////////////////////////////////////////////
INLINE void TextNode::
append_wtext(const wstring &wtext) {
  _wtext = get_wtext() + wtext;
  _flags = (_flags | F_got_wtext) & ~F_got_text;
  rebuild(true);
}

////////////////////////////////////////////////////////////////////
//     Function: TextNode::calc_width
//       Access: Public
//  Description: Returns the width of a line of text of arbitrary
//               characters.  The line should not include the newline
//               character.
////////////////////////////////////////////////////////////////////
INLINE float TextNode::
calc_width(const wstring &line) const {
  nassertr(_font != (TextFont *)NULL, 0.0);
  return _font->calc_width(line);
}

////////////////////////////////////////////////////////////////////
//     Function: TextNode::wordwrap_to
//       Access: Public
//  Description: Inserts newlines into the given text at the
//               appropriate places in order to make each line be the
//               longest possible line that is not longer than
//               wordwrap_width (and does not break any words, if
//               possible).  Returns the new string.
////////////////////////////////////////////////////////////////////
INLINE wstring TextNode::
wordwrap_to(const wstring &wtext, float wordwrap_width,
            bool preserve_trailing_whitespace) const {
  nassertr(_font != (TextFont *)NULL, wtext);
  return _font->wordwrap_to(wtext, wordwrap_width, preserve_trailing_whitespace);
}
