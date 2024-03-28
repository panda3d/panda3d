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
  CDWriter cdata(_cycler);
  cdata->_max_rows = max_rows;
  invalidate_with_measure(cdata);
}

/**
 * Resets the TextNode's default behavior of not limiting the number of rows
 * of text.
 */
INLINE void TextNode::
clear_max_rows() {
  set_max_rows(0);
}

/**
 * Returns true if a limit on the height of the TextNode has been set via
 * set_max_rows(), false otherwise.
 */
INLINE bool TextNode::
has_max_rows() const {
  return get_max_rows() > 0;
}

/**
 * Returns the limit on the height of the TextNode specified by
 * set_max_rows().
 */
INLINE int TextNode::
get_max_rows() const {
  CDReader cdata(_cycler);
  return cdata->_max_rows;
}

/**
 * Returns true if the last text set on the text node exceeded the max_rows
 * constraint, or false if it all fit.
 */
INLINE bool TextNode::
has_overflow() const {
  CDLockedReader cdata(_cycler);
  if (do_needs_measure(cdata)) {
    CDWriter cdataw(((TextNode *)this)->_cycler, cdata, false);
    ((TextNode *)this)->do_measure(cdataw);
    return (cdataw->_flags & F_has_overflow) != 0;
  } else {
    return (cdata->_flags & F_has_overflow) != 0;
  }
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
  CDWriter cdata(_cycler);
  if (cdata->_frame_color != frame_color) {
    cdata->_frame_color = frame_color;
    invalidate_no_measure(cdata);
  }
}

/**
 *
 */
INLINE LColor TextNode::
get_frame_color() const {
  CDReader cdata(_cycler);
  return cdata->_frame_color;
}

/**
 *
 */
INLINE void TextNode::
set_card_border(PN_stdfloat size, PN_stdfloat uv_portion) {
  CDWriter cdata(_cycler);
  if ((cdata->_flags & F_has_card_border) == 0 || cdata->_card_border_size != size || cdata->_card_border_uv_portion != uv_portion) {
    cdata->_flags |= F_has_card_border;
    cdata->_card_border_size = size;
    cdata->_card_border_uv_portion = uv_portion;
    invalidate_no_measure(cdata);
  }
}

/**
 *
 */
INLINE void TextNode::
clear_card_border() {
  CDWriter cdata(_cycler);
  if (cdata->_flags & F_has_card_border) {
    cdata->_flags &= ~F_has_card_border;
    invalidate_no_measure(cdata);
  }
}

/**
 *
 */
INLINE PN_stdfloat TextNode::
get_card_border_size() const {
  CDReader cdata(_cycler);
  return cdata->_card_border_size;
}

/**
 *
 */
INLINE PN_stdfloat TextNode::
get_card_border_uv_portion() const {
  CDReader cdata(_cycler);
  return cdata->_card_border_uv_portion;
}

/**
 *
 */
INLINE bool TextNode::
has_card_border() const {
  CDReader cdata(_cycler);
  return (cdata->_flags & F_has_card_border) != 0;
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
  CDWriter cdata(_cycler);
  if (cdata->_card_color != card_color) {
    cdata->_card_color = card_color;
    invalidate_no_measure(cdata);
  }
}

/**
 *
 */
INLINE LColor TextNode::
get_card_color() const {
  CDReader cdata(_cycler);
  return cdata->_card_color;
}

/**
 *
 */
INLINE void TextNode::
set_card_texture(Texture *card_texture) {
  if (card_texture == nullptr) {
    clear_card_texture();
  } else {
    CDWriter cdata(_cycler);
    if ((cdata->_flags & F_has_card_texture) == 0 || cdata->_card_texture != card_texture) {
      cdata->_flags |= F_has_card_texture;
      cdata->_card_texture = card_texture;
      invalidate_no_measure(cdata);
    }
  }
}

/**
 *
 */
INLINE void TextNode::
clear_card_texture() {
  CDWriter cdata(_cycler);
  if (cdata->_flags & F_has_card_texture) {
    cdata->_flags &= ~F_has_card_texture;
    cdata->_card_texture = nullptr;
    invalidate_no_measure(cdata);
  }
}

/**
 *
 */
INLINE bool TextNode::
has_card_texture() const {
  CDReader cdata(_cycler);
  return (cdata->_flags & F_has_card_texture) != 0;
}

/**
 *
 */
INLINE Texture *TextNode::
get_card_texture() const {
  CDReader cdata(_cycler);
  return cdata->_card_texture;
}

/**
 * Specifies that a border will be drawn around the text when it is next
 * created.  The parameters are the amount of additional padding to insert
 * between the frame and the text in each dimension, and all should generally
 * be positive.
 */
INLINE void TextNode::
set_frame_as_margin(PN_stdfloat left, PN_stdfloat right, PN_stdfloat bottom, PN_stdfloat top) {
  CDWriter cdata(_cycler);
  cdata->_flags |= (F_has_frame | F_frame_as_margin);
  cdata->_frame_ul.set(left, top);
  cdata->_frame_lr.set(right, bottom);
  invalidate_no_measure(cdata);
}

/**
 * Similar to set_frame_as_margin, except the frame is specified in actual
 * coordinate units (relative to the text's origin), irrespective of the size
 * of the text.  The left and bottom coordinates should generally be negative,
 * while the right and top coordinates should generally be positive.
 */
INLINE void TextNode::
set_frame_actual(PN_stdfloat left, PN_stdfloat right, PN_stdfloat bottom, PN_stdfloat top) {
  CDWriter cdata(_cycler);
  cdata->_flags |= F_has_frame;
  cdata->_flags &= ~F_frame_as_margin;
  cdata->_frame_ul.set(left, top);
  cdata->_frame_lr.set(right, bottom);
  invalidate_no_measure(cdata);
}

/**
 * Specifies that a border will not be drawn around the text.
 */
INLINE void TextNode::
clear_frame() {
  CDWriter cdata(_cycler);
  cdata->_flags &= ~F_has_frame;
  invalidate_no_measure(cdata);
}

/**
 *
 */
INLINE bool TextNode::
has_frame() const {
  CDReader cdata(_cycler);
  return (cdata->_flags & F_has_frame) != 0;
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
  CDReader cdata(_cycler);
  nassertr((cdata->_flags & F_has_frame) != 0, false);
  return (cdata->_flags & F_frame_as_margin) != 0;
}

/**
 * Returns the dimensions of the frame as set by set_frame_as_margin() or
 * set_frame_actual().  Use is_frame_actual() to determine how to interpret
 * the values returned by this function.  It is an error to call this if
 * has_frame() is false.
 */
INLINE LVecBase4 TextNode::
get_frame_as_set() const {
  CDReader cdata(_cycler);
  nassertr((cdata->_flags & F_has_frame) != 0, LVecBase4(0.0, 0.0, 0.0, 0.0));
  return LVecBase4(cdata->_frame_ul[0],
                   cdata->_frame_lr[0],
                   cdata->_frame_lr[1],
                   cdata->_frame_ul[1]);
}

/**
 * Specifies the thickness of the lines that will be used to draw the frame.
 */
INLINE void TextNode::
set_frame_line_width(PN_stdfloat frame_width) {
  CDWriter cdata(_cycler);
  cdata->_frame_width = frame_width;
  invalidate_no_measure(cdata);
}

/**
 * Returns the thickness of the lines that will be used to draw the frame.
 */
INLINE PN_stdfloat TextNode::
get_frame_line_width() const {
  CDReader cdata(_cycler);
  return cdata->_frame_width;
}

/**
 * Enables or disables the drawing of corners for the frame.  These are extra
 * points drawn at each of the four corners, to soften the ugly edges
 * generated when the line width is greater than one.
 */
INLINE void TextNode::
set_frame_corners(bool corners) {
  CDWriter cdata(_cycler);
  if (corners) {
    cdata->_flags |= F_frame_corners;
  } else {
    cdata->_flags &= ~F_frame_corners;
  }
  invalidate_no_measure(cdata);
}

/**
 *
 */
INLINE bool TextNode::
get_frame_corners() const {
  CDReader cdata(_cycler);
  return (cdata->_flags & F_frame_corners) != 0;
}

/**
 * Specifies that a (possibly opaque or semitransparent) card will be held
 * behind the text when it is next created.  Like set_frame_as_margin, the
 * parameters are the amount of additional padding to insert around the text
 * in each dimension, and all should generally be positive.
 */
INLINE void TextNode::
set_card_as_margin(PN_stdfloat left, PN_stdfloat right, PN_stdfloat bottom, PN_stdfloat top) {
  CDWriter cdata(_cycler);
  cdata->_flags |= (F_has_card | F_card_as_margin);
  cdata->_card_ul.set(left, top);
  cdata->_card_lr.set(right, bottom);
  invalidate_no_measure(cdata);
}

/**
 * Similar to set_card_as_margin, except the card is specified in actual
 * coordinate units (relative to the text's origin), irrespective of the size
 * of the text.  The left and bottom coordinates should generally be negative,
 * while the right and top coordinates should generally be positive.
 */
INLINE void TextNode::
set_card_actual(PN_stdfloat left, PN_stdfloat right, PN_stdfloat bottom, PN_stdfloat top) {
  CDWriter cdata(_cycler);
  cdata->_flags |= F_has_card;
  cdata->_flags &= ~F_card_as_margin;
  cdata->_card_ul.set(left, top);
  cdata->_card_lr.set(right, bottom);
  invalidate_no_measure(cdata);
}

/**
 * Sets the card_decal flag.  When this is true, the text is decalled onto the
 * card, which is necessary if the TextNode is to be rendered in the 3-d world
 * without putting it in a bin.
 */
INLINE void TextNode::
set_card_decal(bool card_decal) {
  CDWriter cdata(_cycler);
  if (card_decal) {
    cdata->_flags |= F_card_decal;
  } else {
    cdata->_flags &= ~F_card_decal;
  }
  invalidate_no_measure(cdata);
}

/**
 * Specifies that a card will not be drawn behind the text.
 */
INLINE void TextNode::
clear_card() {
  CDWriter cdata(_cycler);
  cdata->_flags &= ~F_has_card;
  invalidate_no_measure(cdata);
}

/**
 *
 */
INLINE bool TextNode::
has_card() const {
  CDReader cdata(_cycler);
  return (cdata->_flags & F_has_card) != 0;
}

/**
 * Returns the card_decal flag.  See set_card_decal().
 */
INLINE bool TextNode::
get_card_decal() const {
  CDReader cdata(_cycler);
  return (cdata->_flags & F_card_decal) != 0;
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
  CDReader cdata(_cycler);
  nassertr((cdata->_flags & F_has_card) != 0, false);
  return (cdata->_flags & F_card_as_margin) != 0;
}

/**
 * Returns the dimensions of the card as set by set_card_as_margin() or
 * set_card_actual().  Use is_card_actual() to determine how to interpret the
 * values returned by this function.  It is an error to call this if
 * has_card() is false.
 */
INLINE LVecBase4 TextNode::
get_card_as_set() const {
  CDReader cdata(_cycler);
  nassertr((cdata->_flags & F_has_card) != 0, LVecBase4(0.0, 0.0, 0.0, 0.0));
  return LVecBase4(cdata->_card_ul[0],
                   cdata->_card_lr[0],
                   cdata->_card_lr[1],
                   cdata->_card_ul[1]);
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

  CDReader cdata(_cycler);
  LPoint3 ul = LPoint3(card[0], 0.0, card[3]) * cdata->_transform;
  LPoint3 lr = LPoint3(card[1], 0.0, card[2]) * cdata->_transform;

  return LVecBase4(ul[0], lr[0], lr[2], ul[2]);
}

/**
 * Sets an additional transform that is applied to the entire text paragraph.
 */
INLINE void TextNode::
set_transform(const LMatrix4 &transform) {
  CDWriter cdata(_cycler);
  cdata->_transform = transform;
  invalidate_with_measure(cdata);
}

/**
 *
 */
INLINE LMatrix4 TextNode::
get_transform() const {
  CDReader cdata(_cycler);
  return cdata->_transform;
}

/**
 * Specifies the coordinate system in which the text will be generated.
 */
INLINE void TextNode::
set_coordinate_system(CoordinateSystem coordinate_system) {
  CDWriter cdata(_cycler);
  cdata->_coordinate_system = coordinate_system;
  invalidate_with_measure(cdata);
}

/**
 *
 */
INLINE CoordinateSystem TextNode::
get_coordinate_system() const {
  CDReader cdata(_cycler);
  return cdata->_coordinate_system;
}

/**
 * Specifies the UsageHint that will be applied to generated geometry.  The
 * default is UH_static, which is probably the right setting, but if you know
 * the TextNode's geometry will have a short lifespan, it may be better to set
 * it to UH_stream.  See geomEnums.h.
 */
INLINE void TextNode::
set_usage_hint(Geom::UsageHint usage_hint) {
  CDWriter cdata(_cycler);
  cdata->_usage_hint = usage_hint;
  invalidate_no_measure(cdata);
}

/**
 * Returns the UsageHint that will be applied to generated geometry.  See
 * set_usage_hint().
 */
INLINE Geom::UsageHint TextNode::
get_usage_hint() const {
  CDReader cdata(_cycler);
  return cdata->_usage_hint;
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
  CDWriter cdata(_cycler);
  cdata->_flatten_flags = flatten_flags;
}

/**
 * Returns the flatten flags.  See set_flatten_flags().
 */
INLINE int TextNode::
get_flatten_flags() const {
  CDReader cdata(_cycler);
  return cdata->_flatten_flags;
}

/**
 * Sets the font that will be used when making text.  If this is set to NULL,
 * the default font will be used, which can be set via set_default_font().
 */
INLINE void TextNode::
set_font(TextFont *font) {
  CDWriter cdata(_cycler, true);
  TextProperties::set_font(font);
  invalidate_with_measure(cdata);
}

/**
 * Resets the font to the default font.
 */
INLINE void TextNode::
clear_font() {
  CDWriter cdata(_cycler, true);
  TextProperties::clear_font();
  invalidate_with_measure(cdata);
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
  CDWriter cdata(_cycler, true);
  TextProperties::set_small_caps(small_caps);
  invalidate_with_measure(cdata);
}

/**
 *
 */
INLINE void TextNode::
clear_small_caps() {
  CDWriter cdata(_cycler);
  TextProperties::clear_small_caps();
  invalidate_with_measure(cdata);
}

/**
 * Sets the scale factor applied to lowercase letters from their uppercase
 * equivalents, when the small_caps flag is in effect.  See set_small_caps().
 * Normally, this will be a number less than one.
 */
INLINE void TextNode::
set_small_caps_scale(PN_stdfloat small_caps_scale) {
  CDWriter cdata(_cycler, true);
  TextProperties::set_small_caps_scale(small_caps_scale);
  invalidate_with_measure(cdata);
}

/**
 *
 */
INLINE void TextNode::
clear_small_caps_scale() {
  CDWriter cdata(_cycler, true);
  TextProperties::clear_small_caps_scale();
  invalidate_with_measure(cdata);
}

/**
 *
 */
INLINE void TextNode::
set_slant(PN_stdfloat slant) {
  CDWriter cdata(_cycler, true);
  TextProperties::set_slant(slant);
  invalidate_with_measure(cdata);
}

/**
 *
 */
INLINE void TextNode::
clear_slant() {
  CDWriter cdata(_cycler, true);
  TextProperties::clear_slant();
  invalidate_with_measure(cdata);
}

/**
 *
 */
INLINE void TextNode::
set_align(TextNode::Alignment align_type) {
  CDWriter cdata(_cycler, true);
  TextProperties::set_align(align_type);
  invalidate_with_measure(cdata);
}

/**
 *
 */
INLINE void TextNode::
clear_align() {
  CDWriter cdata(_cycler, true);
  TextProperties::clear_align();
  invalidate_with_measure(cdata);
}

/**
 * Specifies the amount of extra space that is inserted before the first
 * character of each line.  This can be thought of as a left margin.
 */
INLINE void TextNode::
set_indent(PN_stdfloat indent) {
  CDWriter cdata(_cycler, true);
  TextProperties::set_indent(indent);
  invalidate_with_measure(cdata);
}

/**
 *
 */
INLINE void TextNode::
clear_indent() {
  CDWriter cdata(_cycler, true);
  TextProperties::clear_indent();
  invalidate_with_measure(cdata);
}

/**
 * Sets the text up to automatically wordwrap when it exceeds the indicated
 * width.  This can be thought of as a right margin or margin width.
 */
INLINE void TextNode::
set_wordwrap(PN_stdfloat wordwrap) {
  CDWriter cdata(_cycler, true);
  TextProperties::set_wordwrap(wordwrap);
  invalidate_with_measure(cdata);
}

/**
 * Removes the wordwrap setting from the TextNode.  Text will be as wide as it
 * is.
 */
INLINE void TextNode::
clear_wordwrap() {
  CDWriter cdata(_cycler, true);
  TextProperties::clear_wordwrap();
  invalidate_with_measure(cdata);
}

/**
 * Sets the color of the text.  Note that this will modulate the color of all
 * components of the text, including the shadow and outline.  If you wish to
 * only set the foreground color, see DynamicTextFont::set_fg().
 */
INLINE void TextNode::
set_text_color(const LColor &text_color) {
  CDWriter cdata(_cycler, true);
  TextProperties::set_text_color(text_color);
  invalidate_no_measure(cdata);
}

/**
 * Sets the color of the text.  Note that this will modulate the color of all
 * components of the text, including the shadow and outline.  If you wish to
 * only set the foreground color, see DynamicTextFont::set_fg().
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
  CDWriter cdata(_cycler, true);
  TextProperties::clear_text_color();
  invalidate_no_measure(cdata);
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
  CDWriter cdata(_cycler, true);
  TextProperties::set_shadow_color(shadow_color);
  invalidate_no_measure(cdata);
}

/**
 *
 */
INLINE void TextNode::
clear_shadow_color() {
  CDWriter cdata(_cycler, true);
  TextProperties::clear_shadow_color();
  invalidate_with_measure(cdata);
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
  CDWriter cdata(_cycler, true);
  TextProperties::set_shadow(shadow_offset);
  invalidate_no_measure(cdata);
}

/**
 * Specifies that a shadow will not be drawn behind the text.
 */
INLINE void TextNode::
clear_shadow() {
  CDWriter cdata(_cycler, true);
  TextProperties::clear_shadow();
  invalidate_no_measure(cdata);
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
  CDWriter cdata(_cycler, true);
  TextProperties::set_bin(bin);
  invalidate_no_measure(cdata);
}

/**
 * Removes the effect of a previous call to set_bin().  Text will be drawn in
 * whatever bin it would like to be drawn in, with no explicit ordering.
 */
INLINE void TextNode::
clear_bin() {
  CDWriter cdata(_cycler, true);
  TextProperties::clear_bin();
  invalidate_no_measure(cdata);
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
  CDWriter cdata(_cycler, true);
  invalidate_no_measure(cdata);
  return TextProperties::set_draw_order(draw_order);
}

/**
 *
 */
INLINE void TextNode::
clear_draw_order() {
  CDWriter cdata(_cycler, true);
  TextProperties::clear_draw_order();
  invalidate_with_measure(cdata);
}

/**
 * Sets the width of each tab stop, in screen units.  A tab character embedded
 * in the text will advance the horizontal position to the next tab stop.
 */
INLINE void TextNode::
set_tab_width(PN_stdfloat tab_width) {
  CDWriter cdata(_cycler, true);
  TextProperties::set_tab_width(tab_width);
  invalidate_with_measure(cdata);
}

/**
 *
 */
INLINE void TextNode::
clear_tab_width() {
  CDWriter cdata(_cycler, true);
  TextProperties::clear_tab_width();
  invalidate_with_measure(cdata);
}

/**
 * Specifies the factor by which to scale each letter of the text as it is
 * placed.  This can be used (possibly in conjunction with set_glyph_shift())
 * to implement superscripting or subscripting.
 */
INLINE void TextNode::
set_glyph_scale(PN_stdfloat glyph_scale) {
  CDWriter cdata(_cycler, true);
  TextProperties::set_glyph_scale(glyph_scale);
  invalidate_with_measure(cdata);
}

/**
 *
 */
INLINE void TextNode::
clear_glyph_scale() {
  CDWriter cdata(_cycler, true);
  TextProperties::clear_glyph_scale();
  invalidate_with_measure(cdata);
}

/**
 * Specifies a vertical amount to shift each letter of the text as it is
 * placed.  This can be used (possibly in conjunction with set_glyph_scale())
 * to implement superscripting or subscripting.
 */
INLINE void TextNode::
set_glyph_shift(PN_stdfloat glyph_shift) {
  CDWriter cdata(_cycler, true);
  TextProperties::set_glyph_shift(glyph_shift);
  invalidate_with_measure(cdata);
}

/**
 *
 */
INLINE void TextNode::
clear_glyph_shift() {
  CDWriter cdata(_cycler, true);
  TextProperties::clear_glyph_shift();
  invalidate_with_measure(cdata);
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
  CDLockedReader cdata(_cycler);
  if (do_needs_measure(cdata)) {
    CDWriter cdataw(((TextNode *)this)->_cycler, cdata, false);
    ((TextNode *)this)->do_measure(cdataw);
    return cdataw->_wordwrapped_wtext;
  } else {
    return cdata->_wordwrapped_wtext;
  }
}

/**
 * Returns the leftmost extent of the text in local 2-d coordinates,
 * unmodified by the set_transform() matrix.
 */
INLINE PN_stdfloat TextNode::
get_left() const {
  CDLockedReader cdata(_cycler);
  if (do_needs_measure(cdata)) {
    CDWriter cdataw(((TextNode *)this)->_cycler, cdata, false);
    ((TextNode *)this)->do_measure(cdataw);
    return cdataw->_text_ul[0];
  } else {
    return cdata->_text_ul[0];
  }
}

/**
 * Returns the rightmost extent of the text in local 2-d coordinates,
 * unmodified by the set_transform() matrix.
 */
INLINE PN_stdfloat TextNode::
get_right() const {
  CDLockedReader cdata(_cycler);
  if (do_needs_measure(cdata)) {
    CDWriter cdataw(((TextNode *)this)->_cycler, cdata, false);
    ((TextNode *)this)->do_measure(cdataw);
    return cdataw->_text_lr[0];
  } else {
    return cdata->_text_lr[0];
  }
}

/**
 * Returns the bottommost extent of the text in local 2-d coordinates,
 * unmodified by the set_transform() matrix.
 */
INLINE PN_stdfloat TextNode::
get_bottom() const {
  CDLockedReader cdata(_cycler);
  if (do_needs_measure(cdata)) {
    CDWriter cdataw(((TextNode *)this)->_cycler, cdata, false);
    ((TextNode *)this)->do_measure(cdataw);
    return cdataw->_text_lr[1];
  } else {
    return cdata->_text_lr[1];
  }
}

/**
 * Returns the topmost extent of the text in local 2-d coordinates, unmodified
 * by the set_transform() matrix.
 */
INLINE PN_stdfloat TextNode::
get_top() const {
  CDLockedReader cdata(_cycler);
  if (do_needs_measure(cdata)) {
    CDWriter cdataw(((TextNode *)this)->_cycler, cdata, false);
    ((TextNode *)this)->do_measure(cdataw);
    return cdataw->_text_ul[1];
  } else {
    return cdata->_text_ul[1];
  }
}

/**
 * Returns the net height of the text in local 2-d coordinates.
 */
INLINE PN_stdfloat TextNode::
get_height() const {
  CDLockedReader cdata(_cycler);
  if (do_needs_measure(cdata)) {
    CDWriter cdataw(((TextNode *)this)->_cycler, cdata, false);
    ((TextNode *)this)->do_measure(cdataw);
    return cdataw->_text_ul[1] - cdataw->_text_lr[1];
  } else {
    return cdata->_text_ul[1] - cdata->_text_lr[1];
  }
}

/**
 * Returns the net width of the text in local 2-d coordinates.
 */
INLINE PN_stdfloat TextNode::
get_width() const {
  CDLockedReader cdata(_cycler);
  if (do_needs_measure(cdata)) {
    CDWriter cdataw(((TextNode *)this)->_cycler, cdata, false);
    ((TextNode *)this)->do_measure(cdataw);
    return cdataw->_text_lr[0] - cdataw->_text_ul[0];
  } else {
    return cdata->_text_lr[0] - cdata->_text_ul[0];
  }
}

/**
 * Returns the upper-left extent of the text object, after it has been
 * transformed into 3-d space by applying the set_transform() matrix.
 */
INLINE LPoint3 TextNode::
get_upper_left_3d() const {
  CDLockedReader cdata(_cycler);
  if (do_needs_measure(cdata)) {
    CDWriter cdataw(((TextNode *)this)->_cycler, cdata, false);
    ((TextNode *)this)->do_measure(cdataw);
    return cdataw->_ul3d;
  } else {
    return cdata->_ul3d;
  }
}

/**
 * Returns the lower-right extent of the text object, after it has been
 * transformed into 3-d space by applying the set_transform() matrix.
 */
INLINE LPoint3 TextNode::
get_lower_right_3d() const {
  CDLockedReader cdata(_cycler);
  if (do_needs_measure(cdata)) {
    CDWriter cdataw(((TextNode *)this)->_cycler, cdata, false);
    ((TextNode *)this)->do_measure(cdataw);
    return cdataw->_lr3d;
  } else {
    return cdata->_lr3d;
  }
}

/**
 * Returns the number of rows of text that were generated.  This counts word-
 * wrapped rows as well as rows generated due to embedded newlines.
 */
INLINE int TextNode::
get_num_rows() const {
  CDLockedReader cdata(_cycler);
  if (do_needs_measure(cdata)) {
    CDWriter cdataw(((TextNode *)this)->_cycler, cdata, false);
    ((TextNode *)this)->do_measure(cdataw);
    return cdataw->_num_rows;
  } else {
    return cdata->_num_rows;
  }
}

/**
 * Generates the text, according to the parameters indicated within the
 * TextNode, and returns a Node that may be parented within the tree to
 * represent it.
 */
PT(PandaNode) TextNode::
generate() {
  // Propagate the computed values upstream if the upstream stages have no
  // changes to the text.
  CDWriter cdata(_cycler, false);
  return do_generate(cdata);
}

/**
 * Can be called after the TextNode has been fully configured, to force the
 * node to recompute its text immediately, rather than waiting for it to be
 * drawn.  This call is optional.
 */
INLINE void TextNode::
update() {
  CDLockedReader cdata(_cycler);
  if (do_needs_rebuild(cdata)) {
    // Propagate the generated text upstream if the upstream stages have no
    // changes to the text.
    CDWriter cdataw(_cycler, cdata, false);
    ((TextNode *)this)->do_rebuild(cdataw);
  }
}

/**
 * Forces the TextNode to recompute itself now, even if it believes nothing
 * has changed.  Normally, this should not need to be called, but it may be
 * useful if some properties change outside of the TextNode's knowledge (for
 * instance, within the font).
 */
INLINE void TextNode::
force_update() {
  // Propagate the generated text upstream if the upstream stages have no
  // changes to the text.
  CDWriter cdata(_cycler, false);
  mark_internal_bounds_stale();
  do_rebuild(cdata);
}

/**
 * Called internally whenever some state on the TextNode changes, requiring
 * the internal geometry to be recomputed, but which will not result in a
 * change in the size or shape of the text (for instance, the text color
 * changes).
 */
INLINE void TextNode::
invalidate_no_measure(CData *cdata) {
  cdata->_flags |= F_needs_rebuild;
}

/**
 * Called internally whenever some state on the TextNode changes, requiring
 * the internal geometry to be recomputed, and which will may result in a
 * change in the size or shape of the text (for instance, the text scale
 * changes).
 */
INLINE void TextNode::
invalidate_with_measure(CData *cdata) {
  cdata->_flags |= (F_needs_rebuild | F_needs_measure);
  mark_internal_bounds_stale();
}

/**
 * Returns true if do_rebuild() needs to be called.
 */
INLINE bool TextNode::
do_needs_rebuild(const CData *cdata) const {
  return ((cdata->_flags & F_needs_rebuild) != 0);
}

/**
 * Returns true if do_measure() needs to be called.
 */
INLINE bool TextNode::
do_needs_measure(const CData *cdata) const {
  return ((cdata->_flags & F_needs_measure) != 0);
}
