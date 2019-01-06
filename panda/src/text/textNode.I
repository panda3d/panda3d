/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file textNode.I
 * @author drose
 * @date 2002-03-13
 */

/**
 * Returns the number of units high each line of text is.  This is based on
 * the font.  Note that it is possible for the text to include nested font
 * change commands, in which case the value of this method is questionable.
 */
INLINE PN_stdfloat TextNode::
get_line_height() const {
  TextFont *font = get_font();
  if (font == nullptr) {
    return 0.0f;
  }

  return font->get_line_height();
}

/**
 * Sets the maximum number of rows that may be formatted by the TextNode.  If
 * more text than this is attempted, it will be truncated and has_overflow()
 * will return true.
 */
INLINE void TextNode::
set_max_rows(int max_rows) {
  MutexHolder holder(_lock);
  _max_rows = max_rows;
  invalidate_with_measure();
}

/**
 * Resets the TextNode's default behavior of not limiting the number of rows
 * of text.
 */
INLINE void TextNode::
clear_max_rows() {
  MutexHolder holder(_lock);
  _max_rows = 0;
  invalidate_with_measure();
}

/**
 * Returns true if a limit on the height of the TextNode has been set via
 * set_max_rows(), false otherwise.
 */
INLINE bool TextNode::
has_max_rows() const {
  MutexHolder holder(_lock);
  return _max_rows > 0;
}

/**
 * Returns the limit on the height of the TextNode specified by
 * set_max_rows().
 */
INLINE int TextNode::
get_max_rows() const {
  MutexHolder holder(_lock);
  return _max_rows;
}

/**
 * Returns true if the last text set on the text node exceeded the max_rows
 * constraint, or false if it all fit.
 */
INLINE bool TextNode::
has_overflow() const {
  MutexHolder holder(_lock);
  check_measure();
  return (_flags & F_has_overflow) != 0;
}

/**
 *
 */
INLINE void TextNode::
set_frame_color(PN_stdfloat r, PN_stdfloat g, PN_stdfloat b, PN_stdfloat a) {
  set_frame_color(LColor(r, g, b, a));
}

/**
 *
 */
INLINE void TextNode::
set_frame_color(const LColor &frame_color) {
  MutexHolder holder(_lock);
  if (_frame_color != frame_color) {
    _frame_color = frame_color;
    invalidate_no_measure();
  }
}

/**
 *
 */
INLINE LColor TextNode::
get_frame_color() const {
  MutexHolder holder(_lock);
  return _frame_color;
}

/**
 *
 */
INLINE void TextNode::
set_card_border(PN_stdfloat size, PN_stdfloat uv_portion) {
  MutexHolder holder(_lock);
  if ((_flags & F_has_card_border) == 0 || _card_border_size != size || _card_border_uv_portion != uv_portion) {
    _flags |= F_has_card_border;
    _card_border_size = size;
    _card_border_uv_portion = uv_portion;
    invalidate_no_measure();
  }
}

/**
 *
 */
INLINE void TextNode::
clear_card_border() {
  MutexHolder holder(_lock);
  if (_flags & F_has_card_border) {
    _flags &= ~F_has_card_border;
    invalidate_no_measure();
  }
}

/**
 *
 */
INLINE PN_stdfloat TextNode::
get_card_border_size() const {
  MutexHolder holder(_lock);
  return _card_border_size;
}

/**
 *
 */
INLINE PN_stdfloat TextNode::
get_card_border_uv_portion() const {
  MutexHolder holder(_lock);
  return _card_border_uv_portion;
}

/**
 *
 */
INLINE bool TextNode::
has_card_border() const {
  MutexHolder holder(_lock);
  return (_flags & F_has_card_border) != 0;
}

/**
 *
 */
INLINE void TextNode::
set_card_color(PN_stdfloat r, PN_stdfloat g, PN_stdfloat b, PN_stdfloat a) {
  set_card_color(LColor(r, g, b, a));
}

/**
 *
 */
INLINE void TextNode::
set_card_color(const LColor &card_color) {
  MutexHolder holder(_lock);
  if (_card_color != card_color) {
    _card_color = card_color;
    invalidate_no_measure();
  }
}

/**
 *
 */
INLINE LColor TextNode::
get_card_color() const {
  MutexHolder holder(_lock);
  return _card_color;
}

/**
 *
 */
INLINE void TextNode::
set_card_texture(Texture *card_texture) {
  if (card_texture == nullptr) {
    clear_card_texture();
  } else {
    MutexHolder holder(_lock);
    if ((_flags & F_has_card_texture) == 0 || _card_texture != card_texture) {
      _flags |= F_has_card_texture;
      _card_texture = card_texture;
      invalidate_no_measure();
    }
  }
}

/**
 *
 */
INLINE void TextNode::
clear_card_texture() {
  MutexHolder holder(_lock);
  if (_flags & F_has_card_texture) {
    _flags &= ~F_has_card_texture;
    _card_texture = nullptr;
    invalidate_no_measure();
  }
}

/**
 *
 */
INLINE bool TextNode::
has_card_texture() const {
  MutexHolder holder(_lock);
  return (_flags & F_has_card_texture) != 0;
}

/**
 *
 */
INLINE Texture *TextNode::
get_card_texture() const {
  MutexHolder holder(_lock);
  return _card_texture;
}

/**
 * Specifies that a border will be drawn around the text when it is next
 * created.  The parameters are the amount of additional padding to insert
 * between the frame and the text in each dimension, and all should generally
 * be positive.
 */
INLINE void TextNode::
set_frame_as_margin(PN_stdfloat left, PN_stdfloat right, PN_stdfloat bottom, PN_stdfloat top) {
  MutexHolder holder(_lock);
  _flags |= (F_has_frame | F_frame_as_margin);
  _frame_ul.set(left, top);
  _frame_lr.set(right, bottom);
  invalidate_no_measure();
}

/**
 * Similar to set_frame_as_margin, except the frame is specified in actual
 * coordinate units (relative to the text's origin), irrespective of the size
 * of the text.  The left and bottom coordinates should generally be negative,
 * while the right and top coordinates should generally be positive.
 */
INLINE void TextNode::
set_frame_actual(PN_stdfloat left, PN_stdfloat right, PN_stdfloat bottom, PN_stdfloat top) {
  MutexHolder holder(_lock);
  _flags |= F_has_frame;
  _flags &= ~F_frame_as_margin;
  _frame_ul.set(left, top);
  _frame_lr.set(right, bottom);
  invalidate_no_measure();
}

/**
 * Specifies that a border will not be drawn around the text.
 */
INLINE void TextNode::
clear_frame() {
  MutexHolder holder(_lock);
  _flags &= ~F_has_frame;
  invalidate_no_measure();
}

/**
 *
 */
INLINE bool TextNode::
has_frame() const {
  MutexHolder holder(_lock);
  return (_flags & F_has_frame) != 0;
}

/**
 * If this is true, the frame was set via a call to set_frame_as_margin(), and
 * the dimension of the frame as returned by get_frame_as_set() represent a
 * margin all around the text.  If false, then the frame was set via a call to
 * set_frame_actual(), and the dimensions of the frame as returned by
 * get_frame_as_set() are relative to the text's origin.
 */
INLINE bool TextNode::
is_frame_as_margin() const {
  MutexHolder holder(_lock);
  nassertr((_flags & F_has_frame) != 0, false);
  return (_flags & F_frame_as_margin) != 0;
}

/**
 * Returns the dimensions of the frame as set by set_frame_as_margin() or
 * set_frame_actual().  Use is_frame_actual() to determine how to interpret
 * the values returned by this function.  It is an error to call this if
 * has_frame() is false.
 */
INLINE LVecBase4 TextNode::
get_frame_as_set() const {
  MutexHolder holder(_lock);
  nassertr((_flags & F_has_frame) != 0, LVecBase4(0.0, 0.0, 0.0, 0.0));
  return LVecBase4(_frame_ul[0], _frame_lr[0], _frame_lr[1], _frame_ul[1]);
}

/**
 * Returns the actual dimensions of the frame around the text.  If the frame
 * was set via set_frame_as_margin(), the result returned by this function
 * reflects the size of the current text; if the frame was set via
 * set_frame_actual(), this returns the values actually set.
 *
 * If the text has no frame at all, this returns the dimensions of the text
 * itself, as if the frame were set with a margin of 0, 0, 0, 0.
 */
INLINE LVecBase4 TextNode::
get_frame_actual() const {
  MutexHolder holder(_lock);
  if (_flags & F_has_frame) {
    if (_flags & F_frame_as_margin) {
      check_measure();
      return LVecBase4(_text_ul[0] - _frame_ul[0],
                       _text_lr[0] + _frame_lr[0],
                       _text_lr[1] - _frame_lr[1],
                       _text_ul[1] + _frame_ul[1]);
    } else {
      return LVecBase4(_frame_ul[0], _frame_lr[0], _frame_lr[1], _frame_ul[1]);
    }
  } else {
    check_measure();
    return LVecBase4(_text_ul[0], _text_lr[0], _text_lr[1], _text_ul[1]);
  }
}

/**
 * Specifies the thickness of the lines that will be used to draw the frame.
 */
INLINE void TextNode::
set_frame_line_width(PN_stdfloat frame_width) {
  MutexHolder holder(_lock);
  _frame_width = frame_width;
  invalidate_no_measure();
}

/**
 * Returns the thickness of the lines that will be used to draw the frame.
 */
INLINE PN_stdfloat TextNode::
get_frame_line_width() const {
  MutexHolder holder(_lock);
  return _frame_width;
}

/**
 * Enables or disables the drawing of corners for the frame.  These are extra
 * points drawn at each of the four corners, to soften the ugly edges
 * generated when the line width is greater than one.
 */
INLINE void TextNode::
set_frame_corners(bool corners) {
  MutexHolder holder(_lock);
  if (corners) {
    _flags |= F_frame_corners;
  } else {
    _flags &= ~F_frame_corners;
  }
  invalidate_no_measure();
}

/**
 *
 */
INLINE bool TextNode::
get_frame_corners() const {
  MutexHolder holder(_lock);
  return (_flags & F_frame_corners) != 0;
}

/**
 * Specifies that a (possibly opaque or semitransparent) card will be held
 * behind the text when it is next created.  Like set_frame_as_margin, the
 * parameters are the amount of additional padding to insert around the text
 * in each dimension, and all should generally be positive.
 */
INLINE void TextNode::
set_card_as_margin(PN_stdfloat left, PN_stdfloat right, PN_stdfloat bottom, PN_stdfloat top) {
  MutexHolder holder(_lock);
  _flags |= (F_has_card | F_card_as_margin);
  _card_ul.set(left, top);
  _card_lr.set(right, bottom);
  invalidate_no_measure();
}

/**
 * Similar to set_card_as_margin, except the card is specified in actual
 * coordinate units (relative to the text's origin), irrespective of the size
 * of the text.  The left and bottom coordinates should generally be negative,
 * while the right and top coordinates should generally be positive.
 */
INLINE void TextNode::
set_card_actual(PN_stdfloat left, PN_stdfloat right, PN_stdfloat bottom, PN_stdfloat top) {
  MutexHolder holder(_lock);
  _flags |= F_has_card;
  _flags &= ~F_card_as_margin;
  _card_ul.set(left, top);
  _card_lr.set(right, bottom);
  invalidate_no_measure();
}

/**
 * Sets the card_decal flag.  When this is true, the text is decalled onto the
 * card, which is necessary if the TextNode is to be rendered in the 3-d world
 * without putting it in a bin.
 */
INLINE void TextNode::
set_card_decal(bool card_decal) {
  MutexHolder holder(_lock);
  if (card_decal) {
    _flags |= F_card_decal;
  } else {
    _flags &= ~F_card_decal;
  }
  invalidate_no_measure();
}

/**
 * Specifies that a card will not be drawn behind the text.
 */
INLINE void TextNode::
clear_card() {
  MutexHolder holder(_lock);
  _flags &= ~F_has_card;
  invalidate_no_measure();
}

/**
 *
 */
INLINE bool TextNode::
has_card() const {
  MutexHolder holder(_lock);
  return (_flags & F_has_card) != 0;
}

/**
 * Returns the card_decal flag.  See set_card_decal().
 */
INLINE bool TextNode::
get_card_decal() const {
  MutexHolder holder(_lock);
  return (_flags & F_card_decal) != 0;
}

/**
 * If this is true, the card was set via a call to set_card_as_margin(), and
 * the dimension of the card as returned by get_card_as_set() represent a
 * margin all around the text.  If false, then the card was set via a call to
 * set_card_actual(), and the dimensions of the card as returned by
 * get_card_as_set() are relative to the text's origin.
 */
INLINE bool TextNode::
is_card_as_margin() const {
  MutexHolder holder(_lock);
  nassertr((_flags & F_has_card) != 0, false);
  return (_flags & F_card_as_margin) != 0;
}

/**
 * Returns the dimensions of the card as set by set_card_as_margin() or
 * set_card_actual().  Use is_card_actual() to determine how to interpret the
 * values returned by this function.  It is an error to call this if
 * has_card() is false.
 */
INLINE LVecBase4 TextNode::
get_card_as_set() const {
  MutexHolder holder(_lock);
  nassertr((_flags & F_has_card) != 0, LVecBase4(0.0, 0.0, 0.0, 0.0));
  return LVecBase4(_card_ul[0], _card_lr[0], _card_lr[1], _card_ul[1]);
}

/**
 * Returns the actual dimensions of the card around the text.  If the card was
 * set via set_card_as_margin(), the result returned by this function reflects
 * the size of the current text; if the card was set via set_card_actual(),
 * this returns the values actually set.
 *
 * If the text has no card at all, this returns the dimensions of the text
 * itself, as if the card were set with a margin of 0, 0, 0, 0.
 */
INLINE LVecBase4 TextNode::
get_card_actual() const {
  MutexHolder holder(_lock);
  if (_flags & F_has_card) {
    if (_flags & F_card_as_margin) {
      check_measure();
      return LVecBase4(_text_ul[0] - _card_ul[0],
                       _text_lr[0] + _card_lr[0],
                       _text_lr[1] - _card_lr[1],
                       _text_ul[1] + _card_ul[1]);
    } else {
      return LVecBase4(_card_ul[0], _card_lr[0], _card_lr[1], _card_ul[1]);
    }
  } else {
    check_measure();
    return LVecBase4(_text_ul[0], _text_lr[0], _text_lr[1], _text_ul[1]);
  }
}

/**
 * Returns the actual card dimensions, transformed by the matrix set by
 * set_transform().  This returns the card dimensions in actual coordinates as
 * seen by the rest of the world.  Also see get_upper_left_3d() and
 * get_lower_right_3d().
 */
INLINE LVecBase4 TextNode::
get_card_transformed() const {
  LVecBase4 card = get_card_actual();

  MutexHolder holder(_lock);
  LPoint3 ul = LPoint3(card[0], 0.0, card[3]) * _transform;
  LPoint3 lr = LPoint3(card[1], 0.0, card[2]) * _transform;

  return LVecBase4(ul[0], lr[0], lr[2], ul[2]);
}

/**
 * Sets an additional transform that is applied to the entire text paragraph.
 */
INLINE void TextNode::
set_transform(const LMatrix4 &transform) {
  MutexHolder holder(_lock);
  _transform = transform;
  invalidate_with_measure();
}

/**
 *
 */
INLINE LMatrix4 TextNode::
get_transform() const {
  MutexHolder holder(_lock);
  return _transform;
}

/**
 * Specifies the coordinate system in which the text will be generated.
 */
INLINE void TextNode::
set_coordinate_system(CoordinateSystem coordinate_system) {
  MutexHolder holder(_lock);
  _coordinate_system = coordinate_system;
  invalidate_with_measure();
}

/**
 *
 */
INLINE CoordinateSystem TextNode::
get_coordinate_system() const {
  MutexHolder holder(_lock);
  return _coordinate_system;
}

/**
 * Specifies the UsageHint that will be applied to generated geometry.  The
 * default is UH_static, which is probably the right setting, but if you know
 * the TextNode's geometry will have a short lifespan, it may be better to set
 * it to UH_stream.  See geomEnums.h.
 */
INLINE void TextNode::
set_usage_hint(Geom::UsageHint usage_hint) {
  MutexHolder holder(_lock);
  _usage_hint = usage_hint;
  invalidate_no_measure();
}

/**
 * Returns the UsageHint that will be applied to generated geometry.  See
 * set_usage_hint().
 */
INLINE Geom::UsageHint TextNode::
get_usage_hint() const {
  MutexHolder holder(_lock);
  return _usage_hint;
}

/**
 * Sets the flatten flags.  This should be a union of the
 * TextNode::FlattenFlags options.  This controls the degree of flattening
 * performed on the TextNode's internal geometry (i.e.  the scene graph
 * returned by generate()) each time the text is changed.  In general, more
 * flattening means a more optimal result, but it will take more time to
 * generate.
 *
 * The choice may be any of these three:
 *
 * FF_none - No flatten operation is called.  The letters are left as
 * independent Geoms.
 *
 * FF_light - A flatten_light() operation is called.  The attributes are
 * applied to the vertices, but no nodes are removed.
 *
 * FF_medium - A flatten_medium() operation is called.  The attributes are
 * applied to the vertices, and a few trivial nodes are removed.
 *
 * FF_strong - A flatten_strong() operation is called.  The attributes are
 * applied to the vertices, and the resulting nodes are aggressively combined
 * into as few nodes as possible.
 *
 * In addition to the above choices, you may optionally include the following
 * flag:
 *
 * FF_dynamic_merge - Copy the geoms into a single GeomVertexData as we go,
 * instead of relying on the flatten operation at the end.  This pre-flattens
 * the text considerably, and may obviate the need for flatten altogether; it
 * also tends to improve performance considerably even if you do call flatten.
 * However, it is not as fast as not calling flatten at all.
 *
 * The default is taken from the text-flatten and text-dynamic-merge config
 * variables.
 */
INLINE void TextNode::
set_flatten_flags(int flatten_flags) {
  MutexHolder holder(_lock);
  _flatten_flags = flatten_flags;
}

/**
 * Returns the flatten flags.  See set_flatten_flags().
 */
INLINE int TextNode::
get_flatten_flags() const {
  MutexHolder holder(_lock);
  return _flatten_flags;
}

/**
 * Sets the font that will be used when making text.  If this is set to NULL,
 * the default font will be used, which can be set via set_default_font().
 */
INLINE void TextNode::
set_font(TextFont *font) {
  MutexHolder holder(_lock);
  TextProperties::set_font(font);
  invalidate_with_measure();
}

/**
 * Resets the font to the default font.
 */
INLINE void TextNode::
clear_font() {
  MutexHolder holder(_lock);
  TextProperties::clear_font();
  invalidate_with_measure();
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
INLINE void TextNode::
set_small_caps(bool small_caps) {
  MutexHolder holder(_lock);
  TextProperties::set_small_caps(small_caps);
  invalidate_with_measure();
}

/**
 *
 */
INLINE void TextNode::
clear_small_caps() {
  MutexHolder holder(_lock);
  TextProperties::clear_small_caps();
  invalidate_with_measure();
}

/**
 * Sets the scale factor applied to lowercase letters from their uppercase
 * equivalents, when the small_caps flag is in effect.  See set_small_caps().
 * Normally, this will be a number less than one.
 */
INLINE void TextNode::
set_small_caps_scale(PN_stdfloat small_caps_scale) {
  MutexHolder holder(_lock);
  TextProperties::set_small_caps_scale(small_caps_scale);
  invalidate_with_measure();
}

/**
 *
 */
INLINE void TextNode::
clear_small_caps_scale() {
  MutexHolder holder(_lock);
  TextProperties::clear_small_caps_scale();
  invalidate_with_measure();
}

/**
 *
 */
INLINE void TextNode::
set_slant(PN_stdfloat slant) {
  MutexHolder holder(_lock);
  TextProperties::set_slant(slant);
  invalidate_with_measure();
}

/**
 *
 */
INLINE void TextNode::
clear_slant() {
  MutexHolder holder(_lock);
  TextProperties::clear_slant();
  invalidate_with_measure();
}

/**
 *
 */
INLINE void TextNode::
set_align(TextNode::Alignment align_type) {
  MutexHolder holder(_lock);
  TextProperties::set_align(align_type);
  invalidate_with_measure();
}

/**
 *
 */
INLINE void TextNode::
clear_align() {
  MutexHolder holder(_lock);
  TextProperties::clear_align();
  invalidate_with_measure();
}

/**
 * Specifies the amount of extra space that is inserted before the first
 * character of each line.  This can be thought of as a left margin.
 */
INLINE void TextNode::
set_indent(PN_stdfloat indent) {
  MutexHolder holder(_lock);
  TextProperties::set_indent(indent);
  invalidate_with_measure();
}

/**
 *
 */
INLINE void TextNode::
clear_indent() {
  MutexHolder holder(_lock);
  TextProperties::clear_indent();
  invalidate_with_measure();
}

/**
 * Sets the text up to automatically wordwrap when it exceeds the indicated
 * width.  This can be thought of as a right margin or margin width.
 */
INLINE void TextNode::
set_wordwrap(PN_stdfloat wordwrap) {
  MutexHolder holder(_lock);
  TextProperties::set_wordwrap(wordwrap);
  invalidate_with_measure();
}

/**
 * Removes the wordwrap setting from the TextNode.  Text will be as wide as it
 * is.
 */
INLINE void TextNode::
clear_wordwrap() {
  MutexHolder holder(_lock);
  TextProperties::clear_wordwrap();
  invalidate_with_measure();
}

/**
 *
 */
INLINE void TextNode::
set_text_color(const LColor &text_color) {
  MutexHolder holder(_lock);
  TextProperties::set_text_color(text_color);
  invalidate_no_measure();
}

/**
 *
 */
INLINE void TextNode::
set_text_color(PN_stdfloat r, PN_stdfloat g, PN_stdfloat b, PN_stdfloat a) {
  set_text_color(LColor(r, g, b, a));
}

/**
 * Removes the text color specification; the text will be colored whatever it
 * was in the source font file.
 */
INLINE void TextNode::
clear_text_color() {
  MutexHolder holder(_lock);
  TextProperties::clear_text_color();
  invalidate_no_measure();
}

/**
 *
 */
INLINE void TextNode::
set_shadow_color(PN_stdfloat r, PN_stdfloat g, PN_stdfloat b, PN_stdfloat a) {
  set_shadow_color(LColor(r, g, b, a));
}

/**
 *
 */
INLINE void TextNode::
set_shadow_color(const LColor &shadow_color) {
  MutexHolder holder(_lock);
  TextProperties::set_shadow_color(shadow_color);
  invalidate_no_measure();
}

/**
 *
 */
INLINE void TextNode::
clear_shadow_color() {
  MutexHolder holder(_lock);
  TextProperties::clear_shadow_color();
  invalidate_with_measure();
}

/**
 * Specifies that the text should be drawn with a shadow, by creating a second
 * copy of the text and offsetting it slightly behind the first.
 */
INLINE void TextNode::
set_shadow(PN_stdfloat xoffset, PN_stdfloat yoffset) {
  set_shadow(LVecBase2(xoffset, yoffset));
}

/**
 * Specifies that the text should be drawn with a shadow, by creating a second
 * copy of the text and offsetting it slightly behind the first.
 */
INLINE void TextNode::
set_shadow(const LVecBase2 &shadow_offset) {
  MutexHolder holder(_lock);
  TextProperties::set_shadow(shadow_offset);
  invalidate_no_measure();
}

/**
 * Specifies that a shadow will not be drawn behind the text.
 */
INLINE void TextNode::
clear_shadow() {
  MutexHolder holder(_lock);
  TextProperties::clear_shadow();
  invalidate_no_measure();
}

/**
 * Names the GeomBin that the TextNode geometry should be assigned to.  If
 * this is set, then a GeomBinTransition will be created to explicitly place
 * each component in the named bin.
 *
 * The draw_order value will also be passed to each GeomBinTransition as
 * appropriate; this is particularly useful if this names a GeomBinFixed, e.g.
 * "fixed".
 */
INLINE void TextNode::
set_bin(const std::string &bin) {
  MutexHolder holder(_lock);
  TextProperties::set_bin(bin);
  invalidate_no_measure();
}

/**
 * Removes the effect of a previous call to set_bin().  Text will be drawn in
 * whatever bin it would like to be drawn in, with no explicit ordering.
 */
INLINE void TextNode::
clear_bin() {
  MutexHolder holder(_lock);
  TextProperties::clear_bin();
  invalidate_no_measure();
}

/**
 * Sets the drawing order of text created by the TextMaker.  This is actually
 * the draw order of the card and frame.  The shadow is drawn at
 * _draw_order+1, and the text at _draw_order+2.
 *
 * This affects the sorting order assigned to the arcs as they are created,
 * and also is passed to whatever bin may be assigned via set_bin().
 *
 * The return value is the first unused draw_order number, e.g.  _draw_order +
 * 3.
 */
INLINE int TextNode::
set_draw_order(int draw_order) {
  MutexHolder holder(_lock);
  invalidate_no_measure();
  return TextProperties::set_draw_order(draw_order);
}

/**
 *
 */
INLINE void TextNode::
clear_draw_order() {
  MutexHolder holder(_lock);
  TextProperties::clear_draw_order();
  invalidate_with_measure();
}

/**
 * Sets the width of each tab stop, in screen units.  A tab character embedded
 * in the text will advance the horizontal position to the next tab stop.
 */
INLINE void TextNode::
set_tab_width(PN_stdfloat tab_width) {
  MutexHolder holder(_lock);
  TextProperties::set_tab_width(tab_width);
  invalidate_with_measure();
}

/**
 *
 */
INLINE void TextNode::
clear_tab_width() {
  MutexHolder holder(_lock);
  TextProperties::clear_tab_width();
  invalidate_with_measure();
}

/**
 * Specifies the factor by which to scale each letter of the text as it is
 * placed.  This can be used (possibly in conjunction with set_glyph_shift())
 * to implement superscripting or subscripting.
 */
INLINE void TextNode::
set_glyph_scale(PN_stdfloat glyph_scale) {
  MutexHolder holder(_lock);
  TextProperties::set_glyph_scale(glyph_scale);
  invalidate_with_measure();
}

/**
 *
 */
INLINE void TextNode::
clear_glyph_scale() {
  MutexHolder holder(_lock);
  TextProperties::clear_glyph_scale();
  invalidate_with_measure();
}

/**
 * Specifies a vertical amount to shift each letter of the text as it is
 * placed.  This can be used (possibly in conjunction with set_glyph_scale())
 * to implement superscripting or subscripting.
 */
INLINE void TextNode::
set_glyph_shift(PN_stdfloat glyph_shift) {
  MutexHolder holder(_lock);
  TextProperties::set_glyph_shift(glyph_shift);
  invalidate_with_measure();
}

/**
 *
 */
INLINE void TextNode::
clear_glyph_shift() {
  MutexHolder holder(_lock);
  TextProperties::clear_glyph_shift();
  invalidate_with_measure();
}

/**
 * Returns a string that represents the contents of the text, as it has been
 * formatted by wordwrap rules.
 *
 * In earlier versions, this did not contain any embedded special characters
 * like \1 or \3; now it does.
 */
INLINE std::string TextNode::
get_wordwrapped_text() const {
  return encode_wtext(get_wordwrapped_wtext());
}

/**
 * Returns the width of a line of text of arbitrary characters.  The line
 * should not include the newline character.
 */
INLINE PN_stdfloat TextNode::
calc_width(const std::string &line) const {
  return calc_width(decode_text(line));
}

/**
 * Returns a wstring that represents the contents of the text, as it has been
 * formatted by wordwrap rules.
 *
 * In earlier versions, this did not contain any embedded special characters
 * like \1 or \3; now it does.
 */
INLINE std::wstring TextNode::
get_wordwrapped_wtext() const {
  MutexHolder holder(_lock);
  check_measure();
  return _wordwrapped_wtext;
}

/**
 * Returns the leftmost extent of the text in local 2-d coordinates,
 * unmodified by the set_transform() matrix.
 */
INLINE PN_stdfloat TextNode::
get_left() const {
  MutexHolder holder(_lock);
  check_measure();
  return _text_ul[0];
}

/**
 * Returns the rightmost extent of the text in local 2-d coordinates,
 * unmodified by the set_transform() matrix.
 */
INLINE PN_stdfloat TextNode::
get_right() const {
  MutexHolder holder(_lock);
  check_measure();
  return _text_lr[0];
}

/**
 * Returns the bottommost extent of the text in local 2-d coordinates,
 * unmodified by the set_transform() matrix.
 */
INLINE PN_stdfloat TextNode::
get_bottom() const {
  MutexHolder holder(_lock);
  check_measure();
  return _text_lr[1];
}

/**
 * Returns the topmost extent of the text in local 2-d coordinates, unmodified
 * by the set_transform() matrix.
 */
INLINE PN_stdfloat TextNode::
get_top() const {
  MutexHolder holder(_lock);
  check_measure();
  return _text_ul[1];
}

/**
 * Returns the net height of the text in local 2-d coordinates.
 */
INLINE PN_stdfloat TextNode::
get_height() const {
  MutexHolder holder(_lock);
  check_measure();
  return _text_ul[1] - _text_lr[1];
}

/**
 * Returns the net width of the text in local 2-d coordinates.
 */
INLINE PN_stdfloat TextNode::
get_width() const {
  MutexHolder holder(_lock);
  check_measure();
  return _text_lr[0] - _text_ul[0];
}

/**
 * Returns the upper-left extent of the text object, after it has been
 * transformed into 3-d space by applying the set_transform() matrix.
 */
INLINE LPoint3 TextNode::
get_upper_left_3d() const {
  MutexHolder holder(_lock);
  check_measure();
  return _ul3d;
}

/**
 * Returns the lower-right extent of the text object, after it has been
 * transformed into 3-d space by applying the set_transform() matrix.
 */
INLINE LPoint3 TextNode::
get_lower_right_3d() const {
  MutexHolder holder(_lock);
  check_measure();
  return _lr3d;
}

/**
 * Returns the number of rows of text that were generated.  This counts word-
 * wrapped rows as well as rows generated due to embedded newlines.
 */
INLINE int TextNode::
get_num_rows() const {
  MutexHolder holder(_lock);
  check_measure();
  return _num_rows;
}

/**
 * Generates the text, according to the parameters indicated within the
 * TextNode, and returns a Node that may be parented within the tree to
 * represent it.
 */
PT(PandaNode) TextNode::
generate() {
  MutexHolder holder(_lock);
  return do_generate();
}

/**
 * Can be called after the TextNode has been fully configured, to force the
 * node to recompute its text immediately, rather than waiting for it to be
 * drawn.  This call is optional.
 */
INLINE void TextNode::
update() {
  MutexHolder holder(_lock);
  check_rebuild();
}

/**
 * Forces the TextNode to recompute itself now, even if it believes nothing
 * has changed.  Normally, this should not need to be called, but it may be
 * useful if some properties change outside of the TextNode's knowledge (for
 * instance, within the font).
 */
INLINE void TextNode::
force_update() {
  MutexHolder holder(_lock);
  mark_internal_bounds_stale();
  do_rebuild();
}

/**
 * Called internally whenever some state on the TextNode changes, requiring
 * the internal geometry to be recomputed, but which will not result in a
 * change in the size or shape of the text (for instance, the text color
 * changes).
 */
INLINE void TextNode::
invalidate_no_measure() {
  _flags |= F_needs_rebuild;
}

/**
 * Called internally whenever some state on the TextNode changes, requiring
 * the internal geometry to be recomputed, and which will may result in a
 * change in the size or shape of the text (for instance, the text scale
 * changes).
 */
INLINE void TextNode::
invalidate_with_measure() {
  _flags |= (F_needs_rebuild | F_needs_measure);
  mark_internal_bounds_stale();
}

/**
 * Called internally to call do_rebuild() if necessary (that is, if the
 * internal geometry has changed recently).
 */
INLINE void TextNode::
check_rebuild() const {
  if ((_flags & F_needs_rebuild) != 0) {
    ((TextNode *)this)->do_rebuild();
  }
}

/**
 * Called internally to call do_measure() if necessary; this will remeasure
 * the text without necessarily rebuilding it.
 */
INLINE void TextNode::
check_measure() const {
  if ((_flags & F_needs_measure) != 0) {
    ((TextNode *)this)->do_measure();
  }
}
