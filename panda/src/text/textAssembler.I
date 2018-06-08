/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file textAssembler.I
 * @author drose
 * @date 2004-04-06
 */

/**
 * Specifies the UsageHint that will be applied to generated geometry.  The
 * default is UH_static, which is probably the right setting, but if you know
 * the TextNode's geometry will have a short lifespan, it may be better to set
 * it to UH_stream.  See geomEnums.h.
 */
INLINE void TextAssembler::
set_usage_hint(Geom::UsageHint usage_hint) {
  _usage_hint = usage_hint;
}

/**
 * Returns the UsageHint that will be applied to generated geometry.  See
 * set_usage_hint().
 */
INLINE Geom::UsageHint TextAssembler::
get_usage_hint() const {
  return _usage_hint;
}

/**
 * If max_rows is greater than zero, no more than max_rows will be accepted.
 * Text beyond that will be truncated.
 *
 * Setting this will not truncate text immediately.  You must follow this up
 * with a call to set_wtext() to truncate the existing text.
 */
INLINE void TextAssembler::
set_max_rows(int max_rows) {
  _max_rows = max_rows;
}

/**
 * If max_rows is greater than zero, no more than max_rows will be accepted.
 * Text beyond that will be truncated.
 */
INLINE int TextAssembler::
get_max_rows() const {
  return _max_rows;
}

/**
 * Sets the dynamic_merge flag.  See TextNode::set_flatten_flags().
 */
INLINE void TextAssembler::
set_dynamic_merge(bool dynamic_merge) {
  _dynamic_merge = dynamic_merge;
}

/**
 * Returns the dynamic_merge flag.  See TextNode::set_flatten_flags().
 */
INLINE bool TextAssembler::
get_dynamic_merge() const {
  return _dynamic_merge;
}

/**
 * Sets the multiline mode flag.  Set the multiline mode to allow text to
 * wrap.  It defaults to true.
 */
INLINE void TextAssembler::
set_multiline_mode(bool flag) {
  _multiline_mode = flag;
}

/**
 * Returns the multline_mode flag.  See TextNode::set_multiline_mode().
 */
INLINE bool TextAssembler::
get_multiline_mode() const {
  return _multiline_mode;
}

/**
 * Specifies the default TextProperties that are applied to the text in the
 * absence of any nested property change sequences.
 */
INLINE void TextAssembler::
set_properties(const TextProperties &properties) {
  _initial_cprops = new ComputedProperties(properties);
}

/**
 * Returns the default TextProperties that are applied to the text in the
 * absence of any nested property change sequences.
 */
INLINE const TextProperties &TextAssembler::
get_properties() const {
  return _initial_cprops->_properties;
}

/**
 * Returns the upper-left corner of the assembled text, in 2-d text
 * coordinates.
 */
INLINE const LVector2 &TextAssembler::
get_ul() const {
  return _ul;
}

/**
 * Returns the lower-right corner of the assembled text, in 2-d text
 * coordinates.
 */
INLINE const LVector2 &TextAssembler::
get_lr() const {
  return _lr;
}

/**
 * Computes the row index of the nth character or graphic object in the text
 * and returns it.
 *
 * If the nth character is not a normal printable character with a position in
 * the wordwrapped string, returns -1 (for instance, a soft-hyphen character,
 * or a newline character, may not have a corresponding position).
 */
int TextAssembler::
calc_r(int n) const {
  int r, c;
  if (calc_r_c(r, c, n)) {
    return r;
  }
  return -1;
}

/**
 * Computes the column index of the nth character or graphic object in the
 * text and returns it.
 *
 * If the nth character is not a normal printable character with a position in
 * the wordwrapped string, returns -1 (for instance, a soft-hyphen character,
 * or a newline character, may not have a corresponding position).
 */
int TextAssembler::
calc_c(int n) const {
  int r, c;
  if (calc_r_c(r, c, n)) {
    return c;
  }
  return -1;
}

/**
 * Returns the number of characters of text, before wordwrapping.
 */
INLINE int TextAssembler::
get_num_characters() const {
  return _text_string.size();
}

/**
 * Returns the character at the indicated position in the pre-wordwrapped
 * string.  If the object at this position is a graphic object instead of a
 * character, returns 0.
 */
INLINE wchar_t TextAssembler::
get_character(int n) const {
  nassertr(n >= 0 && n < (int)_text_string.size(), 0);
  return _text_string[n]._character;
}

/**
 * Returns the graphic object at the indicated position in the pre-wordwrapped
 * string.  If the object at this position is a character instead of a graphic
 * object, returns NULL.
 */
INLINE const TextGraphic *TextAssembler::
get_graphic(int n) const {
  nassertr(n >= 0 && n < (int)_text_string.size(), 0);
  return _text_string[n]._graphic;
}

/**
 * Returns the TextProperties in effect for the object at the indicated
 * position in the pre-wordwrapped string.
 */
INLINE const TextProperties &TextAssembler::
get_properties(int n) const {
  nassertr(n >= 0 && n < (int)_text_string.size(), *(new TextProperties()));
  return _text_string[n]._cprops->_properties;
}

/**
 * Returns the width of the character or object at the indicated position in
 * the pre-wordwrapped string.
 */
INLINE PN_stdfloat TextAssembler::
get_width(int n) const {
  nassertr(n >= 0 && n < (int)_text_string.size(), 0.0f);

  return calc_width(_text_string[n]);
}

/**
 * Returns the number of rows of text after it has all been wordwrapped and
 * assembled.
 */
INLINE int TextAssembler::
get_num_rows() const {
  return _text_block.size();
}

/**
 * Returns the number of characters and/or graphic objects in the nth row.
 */
INLINE int TextAssembler::
get_num_cols(int r) const {
  nassertr(r >= 0 && r <= (int)_text_block.size(), 0);
  if (r == (int)_text_block.size()) {
    return 0;
  }
  return _text_block[r]._string.size();
}

/**
 * Returns the character at the indicated position in the indicated row.  If
 * the object at this position is a graphic object instead of a character,
 * returns 0.
 */
INLINE wchar_t TextAssembler::
get_character(int r, int c) const {
  nassertr(r >= 0 && r < (int)_text_block.size(), 0);
  nassertr(c >= 0 && c < (int)_text_block[r]._string.size(), 0);
  return _text_block[r]._string[c]._character;
}

/**
 * Returns the graphic object at the indicated position in the indicated row.
 * If the object at this position is a character instead of a graphic object,
 * returns NULL.
 */
INLINE const TextGraphic *TextAssembler::
get_graphic(int r, int c) const {
  nassertr(r >= 0 && r < (int)_text_block.size(), 0);
  nassertr(c >= 0 && c < (int)_text_block[r]._string.size(), 0);
  return _text_block[r]._string[c]._graphic;
}

/**
 * Returns the TextProperties in effect for the object at the indicated
 * position in the indicated row.
 */
INLINE const TextProperties &TextAssembler::
get_properties(int r, int c) const {
  nassertr(r >= 0 && r < (int)_text_block.size(), *(new TextProperties()));
  nassertr(c >= 0 && c < (int)_text_block[r]._string.size(), *(new TextProperties()));
  return _text_block[r]._string[c]._cprops->_properties;
}

/**
 * Returns the width of the character or object at the indicated position in
 * the indicated row.
 */
INLINE PN_stdfloat TextAssembler::
get_width(int r, int c) const {
  nassertr(r >= 0 && r < (int)_text_block.size(), 0.0f);
  nassertr(c >= 0 && c < (int)_text_block[r]._string.size(), 0.0f);

  return calc_width(_text_block[r]._string[c]);
}

/**
 * Returns the y position of the origin of all of the characters or graphic
 * objects in the indicated row.
 *
 * It is legal for r to exceed the index number of the last row by 1.  The
 * value of c is presently ignored.
 */
INLINE PN_stdfloat TextAssembler::
get_ypos(int r, int) const {
  nassertr(r >= 0 && r <= (int)_text_block.size(), 0.0f);
  if (r == (int)_text_block.size()) {
    return _next_row_ypos;
  } else {
    return _text_block[r]._ypos;
  }
}

/**
 * Returns the width of a single character, according to its associated font.
 */
INLINE PN_stdfloat TextAssembler::
calc_width(const TextCharacter &tch) {
  if (tch._graphic != nullptr) {
    return calc_width(tch._graphic, tch._cprops->_properties);
  } else {
    return calc_width(tch._character, tch._cprops->_properties);
  }
}

/**
 *
 */
INLINE TextAssembler::TextCharacter::
TextCharacter(wchar_t character,
              TextAssembler::ComputedProperties *cprops) :
  _character(character),
  _graphic(nullptr),
  _cprops(cprops)
{
}

/**
 *
 */
INLINE TextAssembler::TextCharacter::
TextCharacter(const TextGraphic *graphic, const std::wstring &graphic_wname,
              TextAssembler::ComputedProperties *cprops) :
  _character(0),
  _graphic(graphic),
  _graphic_wname(graphic_wname),
  _cprops(cprops)
{
}

/**
 *
 */
INLINE TextAssembler::TextCharacter::
TextCharacter(const TextAssembler::TextCharacter &copy) :
  _character(copy._character),
  _graphic(copy._graphic),
  _graphic_wname(copy._graphic_wname),
  _cprops(copy._cprops)
{
}

/**
 *
 */
INLINE void TextAssembler::TextCharacter::
operator = (const TextAssembler::TextCharacter &copy) {
  _character = copy._character;
  _graphic = copy._graphic;
  _graphic_wname = copy._graphic_wname;
  _cprops = copy._cprops;
}

/**
 *
 */
INLINE TextAssembler::TextRow::
TextRow(int row_start) :
  _row_start(row_start),
  _got_soft_hyphens(false),
  _xpos(0.0f),
  _ypos(0.0f)
{
}

/**
 *
 */
INLINE TextAssembler::TextRow::
TextRow(const TextAssembler::TextRow &copy) :
  _string(copy._string),
  _row_start(copy._row_start),
  _got_soft_hyphens(copy._got_soft_hyphens),
  _xpos(copy._xpos),
  _ypos(copy._ypos),
  _eol_cprops(copy._eol_cprops)
{
}

/**
 *
 */
INLINE void TextAssembler::TextRow::
operator = (const TextAssembler::TextRow &copy) {
  _string = copy._string;
  _row_start = copy._row_start;
  _got_soft_hyphens = copy._got_soft_hyphens;
  _xpos = copy._xpos;
  _ypos = copy._ypos;
  _eol_cprops = copy._eol_cprops;
}

/**
 *
 */
INLINE TextAssembler::ComputedProperties::
ComputedProperties(const TextProperties &orig_properties) :
  _based_on(nullptr),
  _depth(0),
  _properties(orig_properties)
{
}

/**
 *
 */
INLINE TextAssembler::ComputedProperties::
ComputedProperties(ComputedProperties *based_on, const std::wstring &wname,
                   TextEncoder *encoder) :
  _based_on(based_on),
  _depth(_based_on->_depth + 1),
  _wname(wname),
  _properties(based_on->_properties)
{
  TextPropertiesManager *manager =
    TextPropertiesManager::get_global_ptr();

  // Now we have to encode the wstring into a string, for lookup in the
  // TextPropertiesManager.
  std::string name = encoder->encode_wtext(wname);

  const TextProperties *named_props = manager->get_properties_ptr(name);
  if (named_props != nullptr) {
    _properties.add_properties(*named_props);
  } else {
    text_cat.warning()
      << "Unknown TextProperties: " << name << "\n";
  }
}

/**
 *
 */
INLINE TextAssembler::GeomCollectorKey::
GeomCollectorKey(const RenderState *state, const GeomVertexFormat *format) :
  _state(state),
  _format(format)
{
}

/**
 *
 */
INLINE bool TextAssembler::GeomCollectorKey::
operator < (const TextAssembler::GeomCollectorKey &other) const {
  if (_state != other._state) {
    return _state < other._state;
  }
  return _format < other._format;
}

/**
 * If the indicated Geom is a GeomTextGlyph, increments its reference count
 * and adds it into this geom.  This is necessary to keep references to
 * outstanding glyphs, so we know when it's safe to recycle no-longer-used
 * glyphs.
 *
 * If the indicated Geom is an ordinary Geom, does nothing.
 */
INLINE void TextAssembler::GeomCollector::
count_geom(const Geom *geom) {
#ifdef HAVE_FREETYPE
  _geom->count_geom(geom);
#endif
}
