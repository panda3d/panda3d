// Filename: textNode.I
// Created by:  mike (04Feb99)
// 
////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////////
//     Function: TextNode::freeze
//       Access: Public
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
  return _freeze_level++;
}

////////////////////////////////////////////////////////////////////
//     Function: TextNode::get_freeze_level
//       Access: Public
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
//       Access: Public
//  Description: Allows changes made since the last freeze() to be
//               visible.  Strictly speaking, this actually decrements
//               the freeze level, and updates the TextNode if the
//               level reaches zero.  The return value is the new
//               freeze level after adjusting.  See freeze().
////////////////////////////////////////////////////////////////////
INLINE int TextNode::
thaw() {
  nassertr(_freeze_level > 0, _freeze_level);
  _freeze_level--;
  
  if (_freeze_level == 0 && _needs_rebuild) {
    do_rebuild();
  }

  return _freeze_level;
}

////////////////////////////////////////////////////////////////////
//     Function: TextNode::set_font
//       Access: Public
//  Description: Sets the font that will be used when making text.
//               This is a model generated via egg-mkfont.
////////////////////////////////////////////////////////////////////
INLINE void TextNode::
set_font(Node *font_def) {
  nassertv(font_def != (Node *)NULL);
  _font = font_def;
  _defs.clear();
  _font_height = 1.0;

  find_characters(font_def);
  rebuild();
}

////////////////////////////////////////////////////////////////////
//     Function: TextNode::get_font
//       Access: Public
//  Description: Returns the font currently in use.
////////////////////////////////////////////////////////////////////
INLINE Node *TextNode::
get_font() const {
  return _font;
}

////////////////////////////////////////////////////////////////////
//     Function: TextNode::get_line_height 
//       Access: Public
//  Description: Returns the number of units high each line of text
//               is.  This is based on the font.
////////////////////////////////////////////////////////////////////
INLINE float TextNode::
get_line_height() const {
  return _font_height;
}

////////////////////////////////////////////////////////////////////
//     Function: TextNode::set_slant
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
INLINE void TextNode::
set_slant(float slant) {
  _slant = slant;
  rebuild();
}

////////////////////////////////////////////////////////////////////
//     Function: TextNode::get_slant
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
INLINE float TextNode::
get_slant() const {
  return _slant;
}

////////////////////////////////////////////////////////////////////
//     Function: TextNode::set_align
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
INLINE void TextNode::
set_align(int align_type) {
  _align = align_type;
  rebuild();
}

////////////////////////////////////////////////////////////////////
//     Function: TextNode::get_align
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
INLINE int TextNode::
get_align() const {
  return _align;
}

////////////////////////////////////////////////////////////////////
//     Function: TextNode::set_wordwrap
//       Access: Public
//  Description: Sets the TextNode up to automatically wordwrap text
//               that exceeds the indicated width.
////////////////////////////////////////////////////////////////////
INLINE void TextNode::
set_wordwrap(float wordwrap) {
  _flags |= F_has_wordwrap;
  _wordwrap_width = wordwrap;
  rebuild();
}

////////////////////////////////////////////////////////////////////
//     Function: TextNode::clear_wordwrap
//       Access: Public
//  Description: Removes the wordwrap setting from the TextNode.  Text
//               will be as wide as it is.
////////////////////////////////////////////////////////////////////
INLINE void TextNode::
clear_wordwrap() {
  _flags &= ~F_has_wordwrap;
  rebuild();
}

////////////////////////////////////////////////////////////////////
//     Function: TextNode::has_wordwrap
//       Access: Public
//  Description: 
////////////////////////////////////////////////////////////////////
INLINE bool TextNode::
has_wordwrap() const {
  return (_flags & F_has_wordwrap) != 0;
}

////////////////////////////////////////////////////////////////////
//     Function: TextNode::get_wordwrap
//       Access: Public
//  Description: 
////////////////////////////////////////////////////////////////////
INLINE float TextNode::
get_wordwrap() const {
  return _wordwrap_width;
}

////////////////////////////////////////////////////////////////////
//     Function: TextNode::set_text_color
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
INLINE void TextNode::
set_text_color(float r, float g, float b, float a) {
  set_text_color(Colorf(r, g, b, a));
}

////////////////////////////////////////////////////////////////////
//     Function: TextNode::set_text_color
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
INLINE void TextNode::
set_text_color(const Colorf &text_color) {
  _text_color = text_color;
  _flags |= F_has_text_color;
  rebuild();
}

////////////////////////////////////////////////////////////////////
//     Function: TextNode::clear_text_color
//       Access: Public
//  Description: Removes the text color specification; the text will
//               be colored whatever it was in the source font file.
////////////////////////////////////////////////////////////////////
INLINE void TextNode::
clear_text_color() {
  _flags &= ~F_has_text_color;
}

////////////////////////////////////////////////////////////////////
//     Function: TextNode::has_text_color
//       Access: Public
//  Description: 
////////////////////////////////////////////////////////////////////
INLINE bool TextNode::
has_text_color() const {
  return (_flags & F_has_text_color) != 0;
}

////////////////////////////////////////////////////////////////////
//     Function: TextNode::get_text_color
//       Access: Public
//  Description: 
////////////////////////////////////////////////////////////////////
INLINE Colorf TextNode::
get_text_color() const {
  return _text_color;
}

////////////////////////////////////////////////////////////////////
//     Function: TextNode::set_frame_color
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
INLINE void TextNode::
set_frame_color(float r, float g, float b, float a) {
  set_frame_color(Colorf(r, g, b, a));
}

////////////////////////////////////////////////////////////////////
//     Function: TextNode::set_frame_color
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
INLINE void TextNode::
set_frame_color(const Colorf &frame_color) {
  _frame_color = frame_color;
  rebuild();
}

////////////////////////////////////////////////////////////////////
//     Function: TextNode::get_frame_color
//       Access: Public
//  Description: 
////////////////////////////////////////////////////////////////////
INLINE Colorf TextNode::
get_frame_color() const {
  return _frame_color;
}

////////////////////////////////////////////////////////////////////
//     Function: TextNode::set_card_border
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
INLINE void TextNode::
set_card_border(float size, float uv_portion) {
  _flags |= F_has_card_border;
  _card_border_size = size;
  _card_border_uv_portion = uv_portion;
  rebuild();
}

////////////////////////////////////////////////////////////////////
//     Function: TextNode::clear_card_border
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
INLINE void TextNode::
clear_card_border() {
  _flags &= ~F_has_card_border;
  rebuild();
}

////////////////////////////////////////////////////////////////////
//     Function: TextNode::get_card_border_size
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
INLINE float TextNode::
get_card_border_size() const {
  return _card_border_size;
}

////////////////////////////////////////////////////////////////////
//     Function: TextNode::get_card_border_uv_portion
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
INLINE float TextNode::
get_card_border_uv_portion() const {
  return _card_border_uv_portion;
}

////////////////////////////////////////////////////////////////////
//     Function: TextNode::has_card_border
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
INLINE bool TextNode::
has_card_border() const {
  return (_flags & F_has_card_border) != 0;
}

////////////////////////////////////////////////////////////////////
//     Function: TextNode::set_card_color
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
INLINE void TextNode::
set_card_color(float r, float g, float b, float a) {
  set_card_color(Colorf(r, g, b, a));
}

////////////////////////////////////////////////////////////////////
//     Function: TextNode::set_card_color
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
INLINE void TextNode::
set_card_color(const Colorf &card_color) {
  _card_color = card_color;
  rebuild();
}

////////////////////////////////////////////////////////////////////
//     Function: TextNode::get_card_color
//       Access: Public
//  Description: 
////////////////////////////////////////////////////////////////////
INLINE Colorf TextNode::
get_card_color() const {
  return _card_color;
}

////////////////////////////////////////////////////////////////////
//     Function: TextNode::set_card_texture
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
INLINE void TextNode::
set_card_texture(Texture *card_texture) {
  _flags |= F_has_card_texture;
  _card_texture = card_texture;
  rebuild();
}

////////////////////////////////////////////////////////////////////
//     Function: TextNode::clear_card_texture
//       Access: Public
//  Description: 
////////////////////////////////////////////////////////////////////
INLINE void TextNode::
clear_card_texture() {
  _flags &= ~F_has_card_texture;
  _card_texture = NULL;
  rebuild();
}

////////////////////////////////////////////////////////////////////
//     Function: TextNode::has_card_texture
//       Access: Public
//  Description: 
////////////////////////////////////////////////////////////////////
INLINE bool TextNode::
has_card_texture() const {
  return (_flags & F_has_card_texture) != 0;
}

////////////////////////////////////////////////////////////////////
//     Function: TextNode::get_card_texture
//       Access: Public
//  Description: 
////////////////////////////////////////////////////////////////////
INLINE Texture *TextNode::
get_card_texture() const {
  return _card_texture;
}

////////////////////////////////////////////////////////////////////
//     Function: TextNode::set_shadow_color
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
INLINE void TextNode::
set_shadow_color(float r, float g, float b, float a) {
  set_shadow_color(Colorf(r, g, b, a));
}

////////////////////////////////////////////////////////////////////
//     Function: TextNode::set_shadow_color
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
INLINE void TextNode::
set_shadow_color(const Colorf &shadow_color) {
  _shadow_color = shadow_color;
  rebuild();
}

////////////////////////////////////////////////////////////////////
//     Function: TextNode::get_shadow_color
//       Access: Public
//  Description: 
////////////////////////////////////////////////////////////////////
INLINE Colorf TextNode::
get_shadow_color() const {
  return _shadow_color;
}

////////////////////////////////////////////////////////////////////
//     Function: TextNode::set_frame_as_margin
//       Access: Public
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
  rebuild();
}

////////////////////////////////////////////////////////////////////
//     Function: TextNode::set_frame_actual
//       Access: Public
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
  rebuild();
}

////////////////////////////////////////////////////////////////////
//     Function: TextNode::clear_frame
//       Access: Public
//  Description: Specifies that a border will not be drawn around the
//               text.
////////////////////////////////////////////////////////////////////
INLINE void TextNode::
clear_frame() {
  _flags &= ~F_has_frame;
  rebuild();
}

////////////////////////////////////////////////////////////////////
//     Function: TextNode::has_frame
//       Access: Public
//  Description: 
////////////////////////////////////////////////////////////////////
INLINE bool TextNode::
has_frame() const {
  return (_flags & F_has_frame) != 0;
}

////////////////////////////////////////////////////////////////////
//     Function: TextNode::is_frame_as_margin
//       Access: Public
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
//       Access: Public
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
//       Access: Public
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
//       Access: Public
//  Description: Specifies the thickness of the lines that will be
//               used to draw the frame.
////////////////////////////////////////////////////////////////////
INLINE void TextNode::
set_frame_line_width(float frame_width) {
  _frame_width = frame_width;
}

////////////////////////////////////////////////////////////////////
//     Function: TextNode::get_frame_line_width
//       Access: Public
//  Description: Returns the thickness of the lines that will be
//               used to draw the frame.
////////////////////////////////////////////////////////////////////
INLINE float TextNode::
get_frame_line_width() const {
  return _frame_width;
}

////////////////////////////////////////////////////////////////////
//     Function: TextNode::set_frame_corners
//       Access: Public
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
//       Access: Public
//  Description: 
////////////////////////////////////////////////////////////////////
INLINE bool TextNode::
get_frame_corners() const {
  return (_flags & F_frame_corners) != 0;
}

////////////////////////////////////////////////////////////////////
//     Function: TextNode::set_card_as_margin
//       Access: Public
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
  rebuild();
}

////////////////////////////////////////////////////////////////////
//     Function: TextNode::set_card_actual
//       Access: Public
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
  rebuild();
}

////////////////////////////////////////////////////////////////////
//     Function: TextNode::clear_card
//       Access: Public
//  Description: Specifies that a card will not be drawn behind the
//               text.
////////////////////////////////////////////////////////////////////
INLINE void TextNode::
clear_card() {
  _flags &= ~F_has_card;
  rebuild();
}

////////////////////////////////////////////////////////////////////
//     Function: TextNode::has_card
//       Access: Public
//  Description: 
////////////////////////////////////////////////////////////////////
INLINE bool TextNode::
has_card() const {
  return (_flags & F_has_card) != 0;
}

////////////////////////////////////////////////////////////////////
//     Function: TextNode::is_card_as_margin
//       Access: Public
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
//       Access: Public
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
//       Access: Public
//  Description: Returns the actual dimensions of the card around the
//               text.  If the card was set via set_card_as_margin(),
//               the result returned by this function reflects the
//               size of the current text; if the card was set via
//               set_card_actual(), this returns the values
//               actually set.
////////////////////////////////////////////////////////////////////
INLINE LVecBase4f TextNode::
get_card_actual() const {
  nassertr(has_card(), LVecBase4f(0.0, 0.0, 0.0, 0.0));
  if (is_card_as_margin()) {
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
//       Access: Public
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
//       Access: Public
//  Description: Specifies that the text should be drawn with a
//               shadow, by creating a second copy of the text and
//               offsetting it slightly behind the first.
////////////////////////////////////////////////////////////////////
INLINE void TextNode::
set_shadow(float xoffset, float yoffset) {
  _flags |= F_has_shadow;
  _shadow_offset.set(xoffset, yoffset);
  rebuild();
}

////////////////////////////////////////////////////////////////////
//     Function: TextNode::clear_shadow
//       Access: Public
//  Description: Specifies that a shadow will not be drawn behind the
//               text.
////////////////////////////////////////////////////////////////////
INLINE void TextNode::
clear_shadow() {
  _flags &= ~F_has_shadow;
  rebuild();
}

////////////////////////////////////////////////////////////////////
//     Function: TextNode::has_shadow
//       Access: Public
//  Description: 
////////////////////////////////////////////////////////////////////
INLINE bool TextNode::
has_shadow() const {
  return (_flags & F_has_shadow) != 0;
}

////////////////////////////////////////////////////////////////////
//     Function: TextNode::get_shadow
//       Access: Public
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
//     Function: TextNode::set_draw_order
//       Access: Public
//  Description: Sets the drawing order of text created by the
//               TextMaker.  This is actually the draw order of the
//               card and frame.  The shadow is drawn at
//               _draw_order+1, and the text at _draw_order+2.
////////////////////////////////////////////////////////////////////
INLINE void TextNode::
set_draw_order(int draw_order) {
  _draw_order = draw_order;
  rebuild();
}

////////////////////////////////////////////////////////////////////
//     Function: TextNode::get_draw_order
//       Access: Public
//  Description: 
////////////////////////////////////////////////////////////////////
INLINE int TextNode::
get_draw_order() const {
  return _draw_order;
}

////////////////////////////////////////////////////////////////////
//     Function: TextNode::set_billboard
//       Access: Public
//  Description: Sets the flag indicating whether the text should be
//               generated as a billboard object or not.  If this is
//               true, the text will automatically billboard.
////////////////////////////////////////////////////////////////////
INLINE void TextNode::
set_billboard(bool billboard) { 
  if (billboard) {
    _flags |= F_billboard;
  } else {
    _flags &= ~F_billboard;
  }
  rebuild();
}

////////////////////////////////////////////////////////////////////
//     Function: TextNode::get_billboard
//       Access: Public
//  Description: 
////////////////////////////////////////////////////////////////////
INLINE bool TextNode::
get_billboard() const {
  return (_flags & F_billboard) != 0;
}

////////////////////////////////////////////////////////////////////
//     Function: TextNode::set_transform
//       Access: Public
//  Description: Sets an additional transform that is applied to the
//               entire text paragraph.
////////////////////////////////////////////////////////////////////
INLINE void TextNode::
set_transform(const LMatrix4f &transform) {
  _transform = transform;
  rebuild();
}

////////////////////////////////////////////////////////////////////
//     Function: TextNode::get_transform
//       Access: Public
//  Description: 
////////////////////////////////////////////////////////////////////
INLINE LMatrix4f TextNode::
get_transform() const {
  return _transform;
}

////////////////////////////////////////////////////////////////////
//     Function: TextNode::set_coordinate_system
//       Access: Public
//  Description: Specifies the coordinate system in which the text
//               will be generated.
////////////////////////////////////////////////////////////////////
INLINE void TextNode::
set_coordinate_system(CoordinateSystem coordinate_system) {
  _coordinate_system = coordinate_system;
  rebuild();
}

////////////////////////////////////////////////////////////////////
//     Function: TextNode::get_coordinate_system
//       Access: Public
//  Description: 
////////////////////////////////////////////////////////////////////
INLINE CoordinateSystem TextNode::
get_coordinate_system() const {
  return _coordinate_system;
}

////////////////////////////////////////////////////////////////////
//     Function: TextNode::set_text
//       Access: Public
//  Description: Changes the text that is displayed under the
//               TextNode.
////////////////////////////////////////////////////////////////////
INLINE void TextNode::
set_text(const string &text) {
  _text = text;
  rebuild();
}

////////////////////////////////////////////////////////////////////
//     Function: TextNode::clear_text
//       Access: Public
//  Description: Removes the text from the TextNode.
////////////////////////////////////////////////////////////////////
INLINE void TextNode::
clear_text() {
  _text = "";
  rebuild();
}

////////////////////////////////////////////////////////////////////
//     Function: TextNode::has_text
//       Access: Public
//  Description: 
////////////////////////////////////////////////////////////////////
INLINE bool TextNode::
has_text() const {
  return !_text.empty();
}

////////////////////////////////////////////////////////////////////
//     Function: TextNode::get_text
//       Access: Public
//  Description: 
////////////////////////////////////////////////////////////////////
INLINE string TextNode::
get_text() const {
  return _text;
}

////////////////////////////////////////////////////////////////////
//     Function: TextNode::rebuild
//       Access: Public
//  Description: Updates the TextNode, if it is not frozen, or marks
//               the TextNode as requiring an update if it is.
//
//               Normally, this function is called automatically
//               whenever any of the parameters changes.  It should
//               not need to be called explicitly unless something
//               goes wrong.
////////////////////////////////////////////////////////////////////
INLINE void TextNode::
rebuild() {
  if (_freeze_level <= 0) {
    do_rebuild();
  } else {
    _needs_rebuild = true;
  }
}

////////////////////////////////////////////////////////////////////
//     Function: TextNode::get_left
//       Access: Public
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
//       Access: Public
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
//       Access: Public
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
//       Access: Public
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
//       Access: Public
//  Description: Returns the net height of the text in local 2-d
//               coordinates.
////////////////////////////////////////////////////////////////////
INLINE float TextNode::
get_height() const {
  return get_top() - get_bottom();
}

////////////////////////////////////////////////////////////////////
//     Function: TextNode::get_width
//       Access: Public
//  Description: Returns the net width of the text in local 2-d
//               coordinates.
////////////////////////////////////////////////////////////////////
INLINE float TextNode::
get_width() const {
  return get_right() - get_left();
}

////////////////////////////////////////////////////////////////////
//     Function: TextNode::get_upper_left_3d
//       Access: Public
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
//       Access: Public
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
//       Access: Public
//  Description: Returns the number of rows of text that were
//               generated.  This counts word-wrapped rows as well as
//               rows generated due to embedded newlines.
////////////////////////////////////////////////////////////////////
INLINE int TextNode::
get_num_rows() const {
  return _num_rows;
}
