// Filename: textNode.I
// Created by:  drose (13Mar02)
//
////////////////////////////////////////////////////////////////////
//
// PANDA 3D SOFTWARE
// Copyright (c) Carnegie Mellon University.  All rights reserved.
//
// All use of this software is subject to the terms of the revised BSD
// license.  You should have received a copy of this license along
// with this source code in a file named "LICENSE."
//
////////////////////////////////////////////////////////////////////


////////////////////////////////////////////////////////////////////
//     Function: TextNode::get_line_height
//       Access: Published
//  Description: Returns the number of units high each line of text
//               is.  This is based on the font.  Note that it is
//               possible for the text to include nested font change
//               commands, in which case the value of this method is
//               questionable.
////////////////////////////////////////////////////////////////////
INLINE PN_stdfloat TextNode::
get_line_height() const {
  TextFont *font = get_font();
  if (font == (TextFont *)NULL) {
    return 0.0f;
  }

  return font->get_line_height();
}

////////////////////////////////////////////////////////////////////
//     Function: TextNode::set_max_rows
//       Access: Published
//  Description: Sets the maximum number of rows that may be formatted
//               by the TextNode.  If more text than this is
//               attempted, it will be truncated and has_overflow()
//               will return true.
////////////////////////////////////////////////////////////////////
INLINE void TextNode::
set_max_rows(int max_rows) {
  _max_rows = max_rows;
  invalidate_with_measure();
}

////////////////////////////////////////////////////////////////////
//     Function: TextNode::clear_max_rows
//       Access: Published
//  Description: Resets the TextNode's default behavior of not
//               limiting the number of rows of text.
////////////////////////////////////////////////////////////////////
INLINE void TextNode::
clear_max_rows() {
  _max_rows = 0;
  invalidate_with_measure();
}

////////////////////////////////////////////////////////////////////
//     Function: TextNode::has_max_rows
//       Access: Published
//  Description: Returns true if a limit on the height of the TextNode
//               has been set via set_max_rows(), false otherwise.
////////////////////////////////////////////////////////////////////
INLINE bool TextNode::
has_max_rows() const {
  return _max_rows > 0;
}

////////////////////////////////////////////////////////////////////
//     Function: TextNode::get_max_rows
//       Access: Published
//  Description: Returns the limit on the height of the TextNode
//               specified by set_max_rows().
////////////////////////////////////////////////////////////////////
INLINE int TextNode::
get_max_rows() const {
  return _max_rows;
}

////////////////////////////////////////////////////////////////////
//     Function: TextNode::has_overflow
//       Access: Published
//  Description: Returns true if the last text set on the text node
//               exceeded the max_rows constraint, or false if it all
//               fit.
////////////////////////////////////////////////////////////////////
INLINE bool TextNode::
has_overflow() const {
  check_measure();
  return (_flags & F_has_overflow) != 0;
}

////////////////////////////////////////////////////////////////////
//     Function: TextNode::set_frame_color
//       Access: Published
//  Description:
////////////////////////////////////////////////////////////////////
INLINE void TextNode::
set_frame_color(PN_stdfloat r, PN_stdfloat g, PN_stdfloat b, PN_stdfloat a) {
  set_frame_color(LColor(r, g, b, a));
}

////////////////////////////////////////////////////////////////////
//     Function: TextNode::set_frame_color
//       Access: Published
//  Description:
////////////////////////////////////////////////////////////////////
INLINE void TextNode::
set_frame_color(const LColor &frame_color) {
  if (_frame_color != frame_color) {
    _frame_color = frame_color;
    invalidate_no_measure();
  }
}

////////////////////////////////////////////////////////////////////
//     Function: TextNode::get_frame_color
//       Access: Published
//  Description:
////////////////////////////////////////////////////////////////////
INLINE LColor TextNode::
get_frame_color() const {
  return _frame_color;
}

////////////////////////////////////////////////////////////////////
//     Function: TextNode::set_card_border
//       Access: Published
//  Description:
////////////////////////////////////////////////////////////////////
INLINE void TextNode::
set_card_border(PN_stdfloat size, PN_stdfloat uv_portion) {
  if (!has_card_border() || _card_border_size != size || _card_border_uv_portion != uv_portion) {
    _flags |= F_has_card_border;
    _card_border_size = size;
    _card_border_uv_portion = uv_portion;
    invalidate_no_measure();
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
    invalidate_no_measure();
  }
}

////////////////////////////////////////////////////////////////////
//     Function: TextNode::get_card_border_size
//       Access: Published
//  Description:
////////////////////////////////////////////////////////////////////
INLINE PN_stdfloat TextNode::
get_card_border_size() const {
  return _card_border_size;
}

////////////////////////////////////////////////////////////////////
//     Function: TextNode::get_card_border_uv_portion
//       Access: Published
//  Description:
////////////////////////////////////////////////////////////////////
INLINE PN_stdfloat TextNode::
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
set_card_color(PN_stdfloat r, PN_stdfloat g, PN_stdfloat b, PN_stdfloat a) {
  set_card_color(LColor(r, g, b, a));
}

////////////////////////////////////////////////////////////////////
//     Function: TextNode::set_card_color
//       Access: Published
//  Description:
////////////////////////////////////////////////////////////////////
INLINE void TextNode::
set_card_color(const LColor &card_color) {
  if (_card_color != card_color) {
    _card_color = card_color;
    invalidate_no_measure();
  }
}

////////////////////////////////////////////////////////////////////
//     Function: TextNode::get_card_color
//       Access: Published
//  Description:
////////////////////////////////////////////////////////////////////
INLINE LColor TextNode::
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
      invalidate_no_measure();
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
    invalidate_no_measure();
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
//     Function: TextNode::set_frame_as_margin
//       Access: Published
//  Description: Specifies that a border will be drawn around the text
//               when it is next created.  The parameters are the
//               amount of additional padding to insert between the
//               frame and the text in each dimension, and all should
//               generally be positive.
////////////////////////////////////////////////////////////////////
INLINE void TextNode::
set_frame_as_margin(PN_stdfloat left, PN_stdfloat right, PN_stdfloat bottom, PN_stdfloat top) {
  _flags |= (F_has_frame | F_frame_as_margin);
  _frame_ul.set(left, top);
  _frame_lr.set(right, bottom);
  invalidate_no_measure();
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
set_frame_actual(PN_stdfloat left, PN_stdfloat right, PN_stdfloat bottom, PN_stdfloat top) {
  _flags |= F_has_frame;
  _flags &= ~F_frame_as_margin;
  _frame_ul.set(left, top);
  _frame_lr.set(right, bottom);
  invalidate_no_measure();
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
  invalidate_no_measure();
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
INLINE LVecBase4 TextNode::
get_frame_as_set() const {
  nassertr(has_frame(), LVecBase4(0.0, 0.0, 0.0, 0.0));
  return LVecBase4(_frame_ul[0], _frame_lr[0], _frame_lr[1], _frame_ul[1]);
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
//
//               If the text has no frame at all, this returns the
//               dimensions of the text itself, as if the frame were
//               set with a margin of 0, 0, 0, 0.
////////////////////////////////////////////////////////////////////
INLINE LVecBase4 TextNode::
get_frame_actual() const {
  if (!has_frame()) {
    check_measure();
    return LVecBase4(_text_ul[0], _text_lr[0], _text_lr[1], _text_ul[1]);

  } else if (is_frame_as_margin()) {
    check_measure();
    return LVecBase4(_text_ul[0] - _frame_ul[0],
                      _text_lr[0] + _frame_lr[0],
                      _text_lr[1] - _frame_lr[1],
                      _text_ul[1] + _frame_ul[1]);
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
set_frame_line_width(PN_stdfloat frame_width) {
  _frame_width = frame_width;
  invalidate_no_measure();
}

////////////////////////////////////////////////////////////////////
//     Function: TextNode::get_frame_line_width
//       Access: Published
//  Description: Returns the thickness of the lines that will be
//               used to draw the frame.
////////////////////////////////////////////////////////////////////
INLINE PN_stdfloat TextNode::
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
  invalidate_no_measure();
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
set_card_as_margin(PN_stdfloat left, PN_stdfloat right, PN_stdfloat bottom, PN_stdfloat top) {
  _flags |= (F_has_card | F_card_as_margin);
  _card_ul.set(left, top);
  _card_lr.set(right, bottom);
  invalidate_no_measure();
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
set_card_actual(PN_stdfloat left, PN_stdfloat right, PN_stdfloat bottom, PN_stdfloat top) {
  _flags |= F_has_card;
  _flags &= ~F_card_as_margin;
  _card_ul.set(left, top);
  _card_lr.set(right, bottom);
  invalidate_no_measure();
}

////////////////////////////////////////////////////////////////////
//     Function: TextNode::set_card_decal
//       Access: Published
//  Description: Sets the card_decal flag.  When this is true, the
//               text is decalled onto the card, which is necessary if
//               the TextNode is to be rendered in the 3-d world
//               without putting it in a bin.
////////////////////////////////////////////////////////////////////
INLINE void TextNode::
set_card_decal(bool card_decal) {
  if (card_decal) {
    _flags |= F_card_decal;
  } else {
    _flags &= ~F_card_decal;
  }
  invalidate_no_measure();
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
  invalidate_no_measure();
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
//     Function: TextNode::get_card_decal
//       Access: Published
//  Description: Returns the card_decal flag.  See set_card_decal().
////////////////////////////////////////////////////////////////////
INLINE bool TextNode::
get_card_decal() const {
  return (_flags & F_card_decal) != 0;
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
INLINE LVecBase4 TextNode::
get_card_as_set() const {
  nassertr(has_card(), LVecBase4(0.0, 0.0, 0.0, 0.0));
  return LVecBase4(_card_ul[0], _card_lr[0], _card_lr[1], _card_ul[1]);
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
INLINE LVecBase4 TextNode::
get_card_actual() const {
  if (!has_card()) {
    check_measure();
    return LVecBase4(_text_ul[0], _text_lr[0], _text_lr[1], _text_ul[1]);

  } else if (is_card_as_margin()) {
    check_measure();
    return LVecBase4(_text_ul[0] - _card_ul[0],
                      _text_lr[0] + _card_lr[0],
                      _text_lr[1] - _card_lr[1],
                      _text_ul[1] + _card_ul[1]);
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
INLINE LVecBase4 TextNode::
get_card_transformed() const {
  LVecBase4 card = get_card_actual();
  LPoint3 ul = LPoint3(card[0], 0.0, card[3]) * _transform;
  LPoint3 lr = LPoint3(card[1], 0.0, card[2]) * _transform;

  return LVecBase4(ul[0], lr[0], lr[2], ul[2]);
}

////////////////////////////////////////////////////////////////////
//     Function: TextNode::set_transform
//       Access: Published
//  Description: Sets an additional transform that is applied to the
//               entire text paragraph.
////////////////////////////////////////////////////////////////////
INLINE void TextNode::
set_transform(const LMatrix4 &transform) {
  _transform = transform;
  invalidate_with_measure();
}

////////////////////////////////////////////////////////////////////
//     Function: TextNode::get_transform
//       Access: Published
//  Description:
////////////////////////////////////////////////////////////////////
INLINE LMatrix4 TextNode::
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
  invalidate_with_measure();
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
//     Function: TextNode::set_usage_hint
//       Access: Published
//  Description: Specifies the UsageHint that will be applied to
//               generated geometry.  The default is UH_static, which
//               is probably the right setting, but if you know the
//               TextNode's geometry will have a short lifespan, it
//               may be better to set it to UH_stream.  See
//               geomEnums.h.
////////////////////////////////////////////////////////////////////
INLINE void TextNode::
set_usage_hint(Geom::UsageHint usage_hint) {
  _usage_hint = usage_hint;
  invalidate_no_measure();
}

////////////////////////////////////////////////////////////////////
//     Function: TextNode::get_usage_hint
//       Access: Published
//  Description: Returns the UsageHint that will be applied to
//               generated geometry.  See set_usage_hint().
////////////////////////////////////////////////////////////////////
INLINE Geom::UsageHint TextNode::
get_usage_hint() const {
  return _usage_hint;
}

////////////////////////////////////////////////////////////////////
//     Function: TextNode::set_flatten_flags
//       Access: Published
//  Description: Sets the flatten flags.  This should be a union of
//               the TextNode::FlattenFlags options.  This controls
//               the degree of flattening performed on the TextNode's
//               internal geometry (i.e. the scene graph returned by
//               generate()) each time the text is changed.  In
//               general, more flattening means a more optimal result,
//               but it will take more time to generate.
//
//               The choice may be any of these three:
//
//               FF_none - No flatten operation is called.  The
//               letters are left as independent Geoms.
//
//               FF_light - A flatten_light() operation is called.
//               The attributes are applied to the vertices, but no
//               nodes are removed.
//
//               FF_medium - A flatten_medium() operation is called.
//               The attributes are applied to the vertices, and a few
//               trivial nodes are removed.
//
//               FF_strong - A flatten_strong() operation is called.
//               The attributes are applied to the vertices, and the
//               resulting nodes are aggressively combined into as few
//               nodes as possible.
//
//               In addition to the above choices, you may optionally
//               include the following flag:
//
//               FF_dynamic_merge - Copy the geoms into a single
//               GeomVertexData as we go, instead of relying on the
//               flatten operation at the end.  This pre-flattens the
//               text considerably, and may obviate the need for
//               flatten altogether; it also tends to improve
//               performance considerably even if you do call flatten.
//               However, it is not as fast as not calling flatten at
//               all.
//
//               The default is taken from the text-flatten and
//               text-dynamic-merge config variables.
////////////////////////////////////////////////////////////////////
INLINE void TextNode::
set_flatten_flags(int flatten_flags) {
  _flatten_flags = flatten_flags;
}

////////////////////////////////////////////////////////////////////
//     Function: TextNode::get_flatten_flags
//       Access: Published
//  Description: Returns the flatten flags.  See set_flatten_flags().
////////////////////////////////////////////////////////////////////
INLINE int TextNode::
get_flatten_flags() const {
  return _flatten_flags;
}

////////////////////////////////////////////////////////////////////
//     Function: TextNode::set_font
//       Access: Published
//  Description: Sets the font that will be used when making text.  If
//               this is set to NULL, the default font will be used,
//               which can be set via set_default_font().
////////////////////////////////////////////////////////////////////
INLINE void TextNode::
set_font(TextFont *font) {
  TextProperties::set_font(font);
  invalidate_with_measure();
}

////////////////////////////////////////////////////////////////////
//     Function: TextNode::clear_font
//       Access: Published
//  Description: Resets the font to the default font.
////////////////////////////////////////////////////////////////////
INLINE void TextNode::
clear_font() {
  TextProperties::clear_font();
  invalidate_with_measure();
}

////////////////////////////////////////////////////////////////////
//     Function: TextNode::set_small_caps
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
INLINE void TextNode::
set_small_caps(bool small_caps) {
  TextProperties::set_small_caps(small_caps);
  invalidate_with_measure();
}

////////////////////////////////////////////////////////////////////
//     Function: TextNode::clear_small_caps
//       Access: Published
//  Description: 
////////////////////////////////////////////////////////////////////
INLINE void TextNode::
clear_small_caps() {
  TextProperties::clear_small_caps();
  invalidate_with_measure();
}

////////////////////////////////////////////////////////////////////
//     Function: TextNode::set_small_caps_scale
//       Access: Published
//  Description: Sets the scale factor applied to lowercase letters
//               from their uppercase equivalents, when the small_caps
//               flag is in effect.  See set_small_caps().  Normally,
//               this will be a number less than one.
////////////////////////////////////////////////////////////////////
INLINE void TextNode::
set_small_caps_scale(PN_stdfloat small_caps_scale) {
  TextProperties::set_small_caps_scale(small_caps_scale);
  invalidate_with_measure();
}

////////////////////////////////////////////////////////////////////
//     Function: TextNode::clear_small_caps_scale
//       Access: Published
//  Description: 
////////////////////////////////////////////////////////////////////
INLINE void TextNode::
clear_small_caps_scale() {
  TextProperties::clear_small_caps_scale();
  invalidate_with_measure();
}

////////////////////////////////////////////////////////////////////
//     Function: TextNode::set_slant
//       Access: Published
//  Description:
////////////////////////////////////////////////////////////////////
INLINE void TextNode::
set_slant(PN_stdfloat slant) {
  TextProperties::set_slant(slant);
  invalidate_with_measure();
}

////////////////////////////////////////////////////////////////////
//     Function: TextNode::clear_slant
//       Access: Published
//  Description: 
////////////////////////////////////////////////////////////////////
INLINE void TextNode::
clear_slant() {
  TextProperties::clear_slant();
  invalidate_with_measure();
}

////////////////////////////////////////////////////////////////////
//     Function: TextNode::set_align
//       Access: Published
//  Description:
////////////////////////////////////////////////////////////////////
INLINE void TextNode::
set_align(TextNode::Alignment align_type) {
  TextProperties::set_align(align_type);
  invalidate_with_measure();
}

////////////////////////////////////////////////////////////////////
//     Function: TextNode::clear_align
//       Access: Published
//  Description: 
////////////////////////////////////////////////////////////////////
INLINE void TextNode::
clear_align() {
  TextProperties::clear_align();
  invalidate_with_measure();
}

////////////////////////////////////////////////////////////////////
//     Function: TextNode::set_indent
//       Access: Published
//  Description: Specifies the amount of extra space that is inserted
//               before the first character of each line.  This can be
//               thought of as a left margin.
////////////////////////////////////////////////////////////////////
INLINE void TextNode::
set_indent(PN_stdfloat indent) {
  TextProperties::set_indent(indent);
  invalidate_with_measure();
}

////////////////////////////////////////////////////////////////////
//     Function: TextNode::clear_indent
//       Access: Published
//  Description: 
////////////////////////////////////////////////////////////////////
INLINE void TextNode::
clear_indent() {
  TextProperties::clear_indent();
  invalidate_with_measure();
}

////////////////////////////////////////////////////////////////////
//     Function: TextNode::set_wordwrap
//       Access: Published
//  Description: Sets the text up to automatically wordwrap when it
//               exceeds the indicated width.  This can be thought of
//               as a right margin or margin width.
////////////////////////////////////////////////////////////////////
INLINE void TextNode::
set_wordwrap(PN_stdfloat wordwrap) {
  TextProperties::set_wordwrap(wordwrap);
  invalidate_with_measure();
}

////////////////////////////////////////////////////////////////////
//     Function: TextNode::clear_wordwrap
//       Access: Published
//  Description: Removes the wordwrap setting from the TextNode.  Text
//               will be as wide as it is.
////////////////////////////////////////////////////////////////////
INLINE void TextNode::
clear_wordwrap() {
  TextProperties::clear_wordwrap();
  invalidate_with_measure();
}

////////////////////////////////////////////////////////////////////
//     Function: TextNode::set_text_color
//       Access: Published
//  Description:
////////////////////////////////////////////////////////////////////
INLINE void TextNode::
set_text_color(const LColor &text_color) {
  TextProperties::set_text_color(text_color);
  invalidate_no_measure();
}

////////////////////////////////////////////////////////////////////
//     Function: TextNode::set_text_color
//       Access: Published
//  Description:
////////////////////////////////////////////////////////////////////
INLINE void TextNode::
set_text_color(PN_stdfloat r, PN_stdfloat g, PN_stdfloat b, PN_stdfloat a) {
  set_text_color(LColor(r, g, b, a));
}

////////////////////////////////////////////////////////////////////
//     Function: TextNode::clear_text_color
//       Access: Published
//  Description: Removes the text color specification; the text will
//               be colored whatever it was in the source font file.
////////////////////////////////////////////////////////////////////
INLINE void TextNode::
clear_text_color() {
  TextProperties::clear_text_color();
  invalidate_no_measure();
}

////////////////////////////////////////////////////////////////////
//     Function: TextNode::set_shadow_color
//       Access: Published
//  Description:
////////////////////////////////////////////////////////////////////
INLINE void TextNode::
set_shadow_color(PN_stdfloat r, PN_stdfloat g, PN_stdfloat b, PN_stdfloat a) {
  set_shadow_color(LColor(r, g, b, a));
}

////////////////////////////////////////////////////////////////////
//     Function: TextNode::set_shadow_color
//       Access: Published
//  Description:
////////////////////////////////////////////////////////////////////
INLINE void TextNode::
set_shadow_color(const LColor &shadow_color) {
  TextProperties::set_shadow_color(shadow_color);
  invalidate_no_measure();
}

////////////////////////////////////////////////////////////////////
//     Function: TextNode::clear_shadow_color
//       Access: Published
//  Description: 
////////////////////////////////////////////////////////////////////
INLINE void TextNode::
clear_shadow_color() {
  TextProperties::clear_shadow_color();
  invalidate_with_measure();
}

////////////////////////////////////////////////////////////////////
//     Function: TextNode::set_shadow
//       Access: Published
//  Description: Specifies that the text should be drawn with a
//               shadow, by creating a second copy of the text and
//               offsetting it slightly behind the first.
////////////////////////////////////////////////////////////////////
INLINE void TextNode::
set_shadow(PN_stdfloat xoffset, PN_stdfloat yoffset) {
  set_shadow(LVecBase2(xoffset, yoffset));
}

////////////////////////////////////////////////////////////////////
//     Function: TextNode::set_shadow
//       Access: Published
//  Description: Specifies that the text should be drawn with a
//               shadow, by creating a second copy of the text and
//               offsetting it slightly behind the first.
////////////////////////////////////////////////////////////////////
INLINE void TextNode::
set_shadow(const LVecBase2 &shadow_offset) {
  TextProperties::set_shadow(shadow_offset);
  invalidate_no_measure();
}

////////////////////////////////////////////////////////////////////
//     Function: TextNode::clear_shadow
//       Access: Published
//  Description: Specifies that a shadow will not be drawn behind the
//               text.
////////////////////////////////////////////////////////////////////
INLINE void TextNode::
clear_shadow() {
  TextProperties::clear_shadow();
  invalidate_no_measure();
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
  TextProperties::set_bin(bin);
  invalidate_no_measure();
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
  TextProperties::clear_bin();
  invalidate_no_measure();
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
  invalidate_no_measure();
  return TextProperties::set_draw_order(draw_order);
}

////////////////////////////////////////////////////////////////////
//     Function: TextNode::clear_draw_order
//       Access: Published
//  Description: 
////////////////////////////////////////////////////////////////////
INLINE void TextNode::
clear_draw_order() {
  TextProperties::clear_draw_order();
  invalidate_with_measure();
}

////////////////////////////////////////////////////////////////////
//     Function: TextNode::set_tab_width
//       Access: Published
//  Description: Sets the width of each tab stop, in screen units.  A
//               tab character embedded in the text will advance the
//               horizontal position to the next tab stop.
////////////////////////////////////////////////////////////////////
INLINE void TextNode::
set_tab_width(PN_stdfloat tab_width) {
  TextProperties::set_tab_width(tab_width);
  invalidate_with_measure();
}

////////////////////////////////////////////////////////////////////
//     Function: TextNode::clear_tab_width
//       Access: Published
//  Description: 
////////////////////////////////////////////////////////////////////
INLINE void TextNode::
clear_tab_width() {
  TextProperties::clear_tab_width();
  invalidate_with_measure();
}

////////////////////////////////////////////////////////////////////
//     Function: TextNode::set_glyph_scale
//       Access: Published
//  Description: Specifies the factor by which to scale each letter of
//               the text as it is placed.  This can be used (possibly
//               in conjunction with set_glyph_shift()) to implement
//               superscripting or subscripting.
////////////////////////////////////////////////////////////////////
INLINE void TextNode::
set_glyph_scale(PN_stdfloat glyph_scale) {
  TextProperties::set_glyph_scale(glyph_scale);
  invalidate_with_measure();
}

////////////////////////////////////////////////////////////////////
//     Function: TextNode::clear_glyph_scale
//       Access: Published
//  Description: 
////////////////////////////////////////////////////////////////////
INLINE void TextNode::
clear_glyph_scale() {
  TextProperties::clear_glyph_scale();
  invalidate_with_measure();
}

////////////////////////////////////////////////////////////////////
//     Function: TextNode::set_glyph_shift
//       Access: Published
//  Description: Specifies a vertical amount to shift each letter of
//               the text as it is placed.  This can be used (possibly
//               in conjunction with set_glyph_scale()) to implement
//               superscripting or subscripting.
////////////////////////////////////////////////////////////////////
INLINE void TextNode::
set_glyph_shift(PN_stdfloat glyph_shift) {
  TextProperties::set_glyph_shift(glyph_shift);
  invalidate_with_measure();
}

////////////////////////////////////////////////////////////////////
//     Function: TextNode::clear_glyph_shift
//       Access: Published
//  Description: 
////////////////////////////////////////////////////////////////////
INLINE void TextNode::
clear_glyph_shift() {
  TextProperties::clear_glyph_shift();
  invalidate_with_measure();
}


////////////////////////////////////////////////////////////////////
//     Function: TextNode::set_text
//       Access: Published
//  Description: Changes the text that is displayed under the
//               TextNode.
////////////////////////////////////////////////////////////////////
INLINE void TextNode::
set_text(const string &text) {
  TextEncoder::set_text(text);
  invalidate_with_measure();
}

////////////////////////////////////////////////////////////////////
//     Function: TextNode::set_text
//       Access: Published
//  Description: The two-parameter version of set_text() accepts an
//               explicit encoding; the text is immediately decoded
//               and stored as a wide-character string.  Subsequent
//               calls to get_text() will return the same text
//               re-encoded using whichever encoding is specified by
//               set_encoding().
////////////////////////////////////////////////////////////////////
INLINE void TextNode::
set_text(const string &text, TextNode::Encoding encoding) {
  TextEncoder::set_text(text, encoding);
  invalidate_with_measure();
}

////////////////////////////////////////////////////////////////////
//     Function: TextNode::clear_text
//       Access: Published
//  Description: Removes the text from the TextNode.
////////////////////////////////////////////////////////////////////
INLINE void TextNode::
clear_text() {
  TextEncoder::clear_text();
  invalidate_with_measure();
}

////////////////////////////////////////////////////////////////////
//     Function: TextNode::append_text
//       Access: Published
//  Description: Appends the indicates string to the end of the stored
//               text.
////////////////////////////////////////////////////////////////////
INLINE void TextNode::
append_text(const string &text) {
  TextEncoder::append_text(text);
  invalidate_with_measure();
}

////////////////////////////////////////////////////////////////////
//     Function: TextNode::append_unicode_char
//       Access: Published
//  Description: Appends a single character to the end of the stored
//               text.  This may be a wide character, up to 16 bits in
//               Unicode.
////////////////////////////////////////////////////////////////////
INLINE void TextNode::
append_unicode_char(wchar_t character) {
  TextEncoder::append_unicode_char(character);
  invalidate_with_measure();
}

////////////////////////////////////////////////////////////////////
//     Function: TextNode::get_wordwrapped_text
//       Access: Public
//  Description: Returns a string that represents the contents of the
//               text, as it has been formatted by wordwrap rules.
//
//               In earlier versions, this did not contain any
//               embedded special characters like \1 or \3; now it
//               does.
////////////////////////////////////////////////////////////////////
INLINE string TextNode::
get_wordwrapped_text() const {
  return encode_wtext(get_wordwrapped_wtext());
}

////////////////////////////////////////////////////////////////////
//     Function: TextNode::calc_width
//       Access: Published
//  Description: Returns the width of a line of text of arbitrary
//               characters.  The line should not include the newline
//               character.
////////////////////////////////////////////////////////////////////
INLINE PN_stdfloat TextNode::
calc_width(const string &line) const {
  return calc_width(decode_text(line));
}

////////////////////////////////////////////////////////////////////
//     Function: TextNode::set_wtext
//       Access: Published
//  Description: Changes the text that is displayed under the
//               TextNode, with a wide text.  This automatically sets
//               the string reported by get_text() to the 8-bit
//               encoded version of the same string.
////////////////////////////////////////////////////////////////////
INLINE void TextNode::
set_wtext(const wstring &wtext) {
  TextEncoder::set_wtext(wtext);
  invalidate_with_measure();
}

////////////////////////////////////////////////////////////////////
//     Function: TextNode::append_wtext
//       Access: Published
//  Description: Appends the indicates string to the end of the stored
//               wide-character text.
////////////////////////////////////////////////////////////////////
INLINE void TextNode::
append_wtext(const wstring &wtext) {
  TextEncoder::append_wtext(wtext);
  invalidate_with_measure();
}

////////////////////////////////////////////////////////////////////
//     Function: TextNode::get_wordwrapped_wtext
//       Access: Published
//  Description: Returns a wstring that represents the contents of the
//               text, as it has been formatted by wordwrap rules.
//
//               In earlier versions, this did not contain any
//               embedded special characters like \1 or \3; now it
//               does.
////////////////////////////////////////////////////////////////////
INLINE wstring TextNode::
get_wordwrapped_wtext() const {
  check_measure();
  return _wordwrapped_wtext;
}

////////////////////////////////////////////////////////////////////
//     Function: TextNode::get_left
//       Access: Published
//  Description: Returns the leftmost extent of the text in local 2-d
//               coordinates, unmodified by the set_transform()
//               matrix.
////////////////////////////////////////////////////////////////////
INLINE PN_stdfloat TextNode::
get_left() const {
  check_measure();
  return _text_ul[0];
}

////////////////////////////////////////////////////////////////////
//     Function: TextNode::get_right
//       Access: Published
//  Description: Returns the rightmost extent of the text in local 2-d
//               coordinates, unmodified by the set_transform()
//               matrix.
////////////////////////////////////////////////////////////////////
INLINE PN_stdfloat TextNode::
get_right() const {
  check_measure();
  return _text_lr[0];
}

////////////////////////////////////////////////////////////////////
//     Function: TextNode::get_bottom
//       Access: Published
//  Description: Returns the bottommost extent of the text in local
//               2-d coordinates, unmodified by the set_transform()
//               matrix.
////////////////////////////////////////////////////////////////////
INLINE PN_stdfloat TextNode::
get_bottom() const {
  check_measure();
  return _text_lr[1];
}

////////////////////////////////////////////////////////////////////
//     Function: TextNode::get_top
//       Access: Published
//  Description: Returns the topmost extent of the text in local 2-d
//               coordinates, unmodified by the set_transform()
//               matrix.
////////////////////////////////////////////////////////////////////
INLINE PN_stdfloat TextNode::
get_top() const {
  check_measure();
  return _text_ul[1];
}

////////////////////////////////////////////////////////////////////
//     Function: TextNode::get_height
//       Access: Published
//  Description: Returns the net height of the text in local 2-d
//               coordinates.
////////////////////////////////////////////////////////////////////
INLINE PN_stdfloat TextNode::
get_height() const {
  check_measure();
  return _text_ul[1] - _text_lr[1];
}

////////////////////////////////////////////////////////////////////
//     Function: TextNode::get_width
//       Access: Published
//  Description: Returns the net width of the text in local 2-d
//               coordinates.
////////////////////////////////////////////////////////////////////
INLINE PN_stdfloat TextNode::
get_width() const {
  check_measure();
  return _text_lr[0] - _text_ul[0];
}

////////////////////////////////////////////////////////////////////
//     Function: TextNode::get_upper_left_3d
//       Access: Published
//  Description: Returns the upper-left extent of the text object,
//               after it has been transformed into 3-d space by
//               applying the set_transform() matrix.
////////////////////////////////////////////////////////////////////
INLINE LPoint3 TextNode::
get_upper_left_3d() const {
  check_measure();
  return _ul3d;
}

////////////////////////////////////////////////////////////////////
//     Function: TextNode::get_lower_right_3d
//       Access: Published
//  Description: Returns the lower-right extent of the text object,
//               after it has been transformed into 3-d space by
//               applying the set_transform() matrix.
////////////////////////////////////////////////////////////////////
INLINE LPoint3 TextNode::
get_lower_right_3d() const {
  check_measure();
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
  check_measure();
  return _num_rows;
}

////////////////////////////////////////////////////////////////////
//     Function: TextNode::update
//       Access: Published
//  Description: Can be called after the TextNode has been fully
//               configured, to force the node to recompute its text
//               immediately, rather than waiting for it to be drawn.
//               This call is optional.
////////////////////////////////////////////////////////////////////
INLINE void TextNode::
update() {
  check_rebuild();
}

////////////////////////////////////////////////////////////////////
//     Function: TextNode::force_update
//       Access: Published
//  Description: Forces the TextNode to recompute itself now, even if
//               it believes nothing has changed.  Normally, this
//               should not need to be called, but it may be useful if
//               some properties change outside of the TextNode's
//               knowledge (for instance, within the font).
////////////////////////////////////////////////////////////////////
INLINE void TextNode::
force_update() {
  invalidate_with_measure();
  check_rebuild();
}

////////////////////////////////////////////////////////////////////
//     Function: TextNode::invalidate_no_measure
//       Access: Private
//  Description: Called internally whenever some state on the TextNode
//               changes, requiring the internal geometry to be
//               recomputed, but which will not result in a change in
//               the size or shape of the text (for instance, the text
//               color changes).
////////////////////////////////////////////////////////////////////
INLINE void TextNode::
invalidate_no_measure() {
  _flags |= F_needs_rebuild;
}

////////////////////////////////////////////////////////////////////
//     Function: TextNode::invalidate_with_measure
//       Access: Private
//  Description: Called internally whenever some state on the TextNode
//               changes, requiring the internal geometry to be
//               recomputed, and which will may result in a change in
//               the size or shape of the text (for instance, the text
//               scale changes).
////////////////////////////////////////////////////////////////////
INLINE void TextNode::
invalidate_with_measure() {
  _flags |= (F_needs_rebuild | F_needs_measure);
  mark_internal_bounds_stale();
}

////////////////////////////////////////////////////////////////////
//     Function: TextNode::check_rebuild
//       Access: Private
//  Description: Called internally to call do_rebuild() if necessary
//               (that is, if the internal geometry has changed
//               recently).
////////////////////////////////////////////////////////////////////
INLINE void TextNode::
check_rebuild() const {
  if ((_flags & F_needs_rebuild) != 0) {
    ((TextNode *)this)->do_rebuild();
  }
}

////////////////////////////////////////////////////////////////////
//     Function: TextNode::check_measure
//       Access: Private
//  Description: Called internally to call do_measure() if necessary;
//               this will remeasure the text without necessarily
//               rebuilding it.
////////////////////////////////////////////////////////////////////
INLINE void TextNode::
check_measure() const {
  if ((_flags & F_needs_measure) != 0) {
    ((TextNode *)this)->do_measure();
  }
}
