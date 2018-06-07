/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file pgEntry.I
 * @author drose
 * @date 2002-03-13
 */

/**
 * Changes the text currently displayed within the entry.  This uses the
 * Unicode encoding currently specified for the "focus" TextNode; therefore,
 * the TextNode must exist before calling set_text().
 *
 * The return value is true if all the text is accepted, or false if some was
 * truncated (see set_max_width(), etc.).
 */
INLINE bool PGEntry::
set_text(const std::string &text) {
  LightReMutexHolder holder(_lock);
  TextNode *text_node = get_text_def(S_focus);
  nassertr(text_node != nullptr, false);
  return set_wtext(text_node->decode_text(text));
}

/**
 * Returns the text currently displayed within the entry, without any embedded
 * properties characters.
 *
 * This uses the Unicode encoding currently specified for the "focus"
 * TextNode; therefore, the TextNode must exist before calling get_text().
 */
INLINE std::string PGEntry::
get_plain_text() const {
  LightReMutexHolder holder(_lock);
  TextNode *text_node = get_text_def(S_focus);
  nassertr(text_node != nullptr, std::string());
  return text_node->encode_wtext(get_plain_wtext());
}

/**
 * Returns the text currently displayed within the entry.  This uses the
 * Unicode encoding currently specified for the "focus" TextNode; therefore,
 * the TextNode must exist before calling get_text().
 */
INLINE std::string PGEntry::
get_text() const {
  LightReMutexHolder holder(_lock);
  TextNode *text_node = get_text_def(S_focus);
  nassertr(text_node != nullptr, std::string());
  return text_node->encode_wtext(get_wtext());
}

/**
 * Returns the number of characters of text in the entry.  This is the actual
 * number of visible characters, not counting implicit newlines due to
 * wordwrapping, or formatted characters for text properties changes.  If
 * there is an embedded TextGraphic object, it counts as one character.
 *
 * This is also the length of the string returned by get_plain_text().
 */
INLINE int PGEntry::
get_num_characters() const {
  LightReMutexHolder holder(_lock);
  return _text.get_num_characters();
}

/**
 * Returns the character at the indicated position in the entry.  If the
 * object at this position is a graphic object instead of a character, returns
 * 0.
 */
INLINE wchar_t PGEntry::
get_character(int n) const {
  LightReMutexHolder holder(_lock);
  return _text.get_character(n);
}

/**
 * Returns the graphic object at the indicated position in the pre-wordwrapped
 * string.  If the object at this position is a character instead of a graphic
 * object, returns NULL.
 */
INLINE const TextGraphic *PGEntry::
get_graphic(int n) const {
  LightReMutexHolder holder(_lock);
  return _text.get_graphic(n);
}

/**
 * Returns the TextProperties in effect for the object at the indicated
 * position in the pre-wordwrapped string.
 */
INLINE const TextProperties &PGEntry::
get_properties(int n) const {
  LightReMutexHolder holder(_lock);
  return _text.get_properties(n);
}

/**
 * Sets the current position of the cursor.  This is the position within the
 * text at which the next letter typed by the user will be inserted; normally
 * it is the same as the length of the text.
 */
INLINE void PGEntry::
set_cursor_position(int position) {
  LightReMutexHolder holder(_lock);
  if (_cursor_position != position) {
    _cursor_position = position;
    _cursor_stale = true;
    _blink_start = ClockObject::get_global_clock()->get_frame_time();
  }
}

/**
 * Returns the current position of the cursor.
 */
INLINE int PGEntry::
get_cursor_position() const {
  LightReMutexHolder holder(_lock);
  return _cursor_position;
}

/**
 * Returns the node position x of the cursor
 */

INLINE PN_stdfloat PGEntry::
get_cursor_X() const {
  LightReMutexHolder holder(_lock);
  return _cursor_def.get_x();
}

/**
 * Returns the node position y of the cursor
 */

INLINE PN_stdfloat PGEntry::
get_cursor_Y() const {
  LightReMutexHolder holder(_lock);
  return _cursor_def.get_y();
}


/**
 * Sets the maximum number of characters that may be typed into the entry.
 * This is a limit on the number of characters, as opposed to the width of the
 * entry; see also set_max_width().
 *
 * If this is 0, there is no limit.
 */
INLINE void PGEntry::
set_max_chars(int max_chars) {
  LightReMutexHolder holder(_lock);
  _max_chars = max_chars;
}

/**
 * Returns the current maximum number of characters that may be typed into the
 * entry, or 0 if there is no limit.  See set_max_chars().
 */
INLINE int PGEntry::
get_max_chars() const {
  LightReMutexHolder holder(_lock);
  return _max_chars;
}

/**
 * Sets the maximum width of all characters that may be typed into the entry.
 * This is a limit on the width of the formatted text, not a fixed limit on
 * the number of characters; also set_max_chars().
 *
 * If this is 0, there is no limit.
 *
 * If _num_lines is more than 1, rather than being a fixed width on the whole
 * entry, this becomes instead the wordwrap width (and the width limit on the
 * entry is essentially _max_width * _num_lines).
 */
INLINE void PGEntry::
set_max_width(PN_stdfloat max_width) {
  LightReMutexHolder holder(_lock);
  _max_width = max_width;
  _text_geom_stale = true;
}

/**
 * Returns the current maximum width of the characters that may be typed into
 * the entry, or 0 if there is no limit.  See set_max_width().
 */
INLINE PN_stdfloat PGEntry::
get_max_width() const {
  LightReMutexHolder holder(_lock);
  return _max_width;
}

/**
 * Sets the number of lines of text the PGEntry will use.  This only has
 * meaning if _max_width is not 0; _max_width indicates the wordwrap width of
 * each line.
 */
INLINE void PGEntry::
set_num_lines(int num_lines) {
  LightReMutexHolder holder(_lock);
  nassertv(num_lines >= 1);
  _num_lines = num_lines;
  _text_geom_stale = true;
  update_text();
}

/**
 * Returns the number of lines of text the PGEntry will use, if _max_width is
 * not 0.  See set_num_lines().
 */
INLINE int PGEntry::
get_num_lines() const {
  LightReMutexHolder holder(_lock);
  return _num_lines;
}

/**
 * Sets the number of times per second the cursor will blink while the entry
 * has keyboard focus.
 *
 * If this is 0, the cursor does not blink, but is held steady.
 */
INLINE void PGEntry::
set_blink_rate(PN_stdfloat blink_rate) {
  LightReMutexHolder holder(_lock);
  _blink_rate = blink_rate;
}

/**
 * Returns the number of times per second the cursor will blink, or 0 if the
 * cursor is not to blink.
 */
INLINE PN_stdfloat PGEntry::
get_blink_rate() const {
  LightReMutexHolder holder(_lock);
  return _blink_rate;
}

/**
 * Returns the Node that will be rendered to represent the cursor.  You can
 * attach suitable cursor geometry to this node.
 */
INLINE NodePath PGEntry::
get_cursor_def() {
  LightReMutexHolder holder(_lock);
  return _cursor_def;
}

/**
 * Removes all the children from the cursor_def node, in preparation for
 * adding a new definition.
 */
INLINE void PGEntry::
clear_cursor_def() {
  LightReMutexHolder holder(_lock);
  _cursor_def.remove_node();
  _cursor_def = _cursor_scale.attach_new_node("cursor");
}

/**
 * Sets whether the arrow keys (and home/end) control movement of the cursor.
 * If true, they are active; if false, they are ignored.
 */
INLINE void PGEntry::
set_cursor_keys_active(bool flag) {
  LightReMutexHolder holder(_lock);
  _cursor_keys_active = flag;
}

/**
 * Returns whether the arrow keys are currently set to control movement of the
 * cursor; see set_cursor_keys_active().
 */
INLINE bool PGEntry::
get_cursor_keys_active() const {
  LightReMutexHolder holder(_lock);
  return _cursor_keys_active;
}

/**
 * Specifies whether obscure mode should be enabled.  In obscure mode, a
 * string of asterisks is displayed instead of the literal text, e.g.  for
 * entering passwords.
 *
 * In obscure mode, the width of the text is computed based on the width of
 * the string of asterisks, not on the width of the actual text.  This has
 * implications on the maximum length of text that may be entered if max_width
 * is in effect.
 */
INLINE void PGEntry::
set_obscure_mode(bool flag) {
  LightReMutexHolder holder(_lock);
  if (_obscure_mode != flag) {
    _obscure_mode = flag;
    _text_geom_stale = true;
  }
}

/**
 * Specifies whether obscure mode is enabled.  See set_obscure_mode().
 */
INLINE bool PGEntry::
get_obscure_mode() const {
  LightReMutexHolder holder(_lock);
  return _obscure_mode;
}

/**
 * Specifies whether overflow mode should be enabled.  In overflow mode, text
 * can overflow the boundaries of the Entry element horizontally.
 *
 * Overflow mode only works when the number of lines is 1.
 */
INLINE void PGEntry::
set_overflow_mode(bool flag) {
  LightReMutexHolder holder(_lock);
  if (_overflow_mode != flag) {
    _overflow_mode = flag;
    _text_geom_stale = true;
    _cursor_stale = true;
  }
}

/**
 * Specifies whether overflow mode is enabled.  See set_overflow_mode().
 */
INLINE bool PGEntry::
get_overflow_mode() const {
  LightReMutexHolder holder(_lock);
  return _overflow_mode;
}

/**
 * Specifies the name of the TextProperties structure added to the
 * TextPropertiesManager that will be used to render candidate strings from
 * the IME, used for typing characters in east Asian languages.  Each
 * candidate string represents one possible way to interpret the sequence of
 * keys the user has just entered; it should not be considered typed yet, but
 * it is important for the user to be able to see what he is considering
 * entering.
 *
 * This particular method sets the properties for the subset of the current
 * candidate string that the user can actively scroll through.
 */
INLINE void PGEntry::
set_candidate_active(const std::string &candidate_active) {
  LightReMutexHolder holder(_lock);
  _candidate_active = candidate_active;
}

/**
 * See set_candidate_active().
 */
INLINE const std::string &PGEntry::
get_candidate_active() const {
  LightReMutexHolder holder(_lock);
  return _candidate_active;
}

/**
 * Specifies the name of the TextProperties structure added to the
 * TextPropertiesManager that will be used to render candidate strings from
 * the IME, used for typing characters in east Asian languages.  Each
 * candidate string represents one possible way to interpret the sequence of
 * keys the user has just entered; it should not be considered typed yet, but
 * it is important for the user to be able to see what he is considering
 * entering.
 *
 * This particular method sets the properties for the subset of the current
 * candidate string that the user is not actively scrolling through.
 */
INLINE void PGEntry::
set_candidate_inactive(const std::string &candidate_inactive) {
  LightReMutexHolder holder(_lock);
  _candidate_inactive = candidate_inactive;
}

/**
 * See set_candidate_inactive().
 */
INLINE const std::string &PGEntry::
get_candidate_inactive() const {
  LightReMutexHolder holder(_lock);
  return _candidate_inactive;
}

/**
 * Returns the prefix that is used to define the accept event for all
 * PGEntries.  The accept event is the concatenation of this string followed
 * by get_id().
 */
INLINE std::string PGEntry::
get_accept_prefix() {
  return "accept-";
}

/**
 * Returns the prefix that is used to define the accept failed event for all
 * PGEntries.  This event is the concatenation of this string followed by
 * get_id().
 */
INLINE std::string PGEntry::
get_accept_failed_prefix() {
  return "acceptfailed-";
}

/**
 * Returns the prefix that is used to define the overflow event for all
 * PGEntries.  The overflow event is the concatenation of this string followed
 * by get_id().
 */
INLINE std::string PGEntry::
get_overflow_prefix() {
  return "overflow-";
}

/**
 * Returns the prefix that is used to define the type event for all PGEntries.
 * The type event is the concatenation of this string followed by get_id().
 */
INLINE std::string PGEntry::
get_type_prefix() {
  return "type-";
}

/**
 * Returns the prefix that is used to define the erase event for all
 * PGEntries.  The erase event is the concatenation of this string followed by
 * get_id().
 */
INLINE std::string PGEntry::
get_erase_prefix() {
  return "erase-";
}

/**
 * Returns the prefix that is used to define the cursor event for all
 * PGEntries.  The cursor event is the concatenation of this string followed
 * by get_id().
 */
INLINE std::string PGEntry::
get_cursormove_prefix() {
  return "cursormove-";
}

/**
 * Returns the event name that will be thrown when the entry is accepted
 * normally.
 */
INLINE std::string PGEntry::
get_accept_event(const ButtonHandle &button) const {
  return get_accept_prefix() + button.get_name() + "-" + get_id();
}

/**
 * Returns the event name that will be thrown when the entry cannot accept an
 * input
 */
INLINE std::string PGEntry::
get_accept_failed_event(const ButtonHandle &button) const {
  return get_accept_failed_prefix() + button.get_name() + "-" + get_id();
}

/**
 * Returns the event name that will be thrown when too much text is attempted
 * to be entered into the PGEntry, exceeding either the limit set via
 * set_max_chars() or via set_max_width().
 */
INLINE std::string PGEntry::
get_overflow_event() const {
  return get_overflow_prefix() + get_id();
}

/**
 * Returns the event name that will be thrown whenever the user extends the
 * text by typing.
 */
INLINE std::string PGEntry::
get_type_event() const {
  return get_type_prefix() + get_id();
}

/**
 * Returns the event name that will be thrown whenever the user erases
 * characters in the text.
 */
INLINE std::string PGEntry::
get_erase_event() const {
  return get_erase_prefix() + get_id();
}

/**
 * Returns the event name that will be thrown whenever the cursor moves
 */
INLINE std::string PGEntry::
get_cursormove_event() const {
  return get_cursormove_prefix() + get_id();
}

/**
 * Changes the text currently displayed within the entry.
 *
 * The return value is true if all the text is accepted, or false if some was
 * truncated (see set_max_width(), etc.).
 */
INLINE bool PGEntry::
set_wtext(const std::wstring &wtext) {
  LightReMutexHolder holder(_lock);
  bool ret = _text.set_wtext(wtext);
  if (_obscure_mode) {
    ret = _obscure_text.set_wtext(std::wstring(_text.get_num_characters(), '*'));
  }
  _text_geom_stale = true;
  set_cursor_position(std::min(_cursor_position, _text.get_num_characters()));
  return ret;
}

/**
 * Returns the text currently displayed within the entry, without any embedded
 * properties characters.
 */
INLINE std::wstring PGEntry::
get_plain_wtext() const {
  LightReMutexHolder holder(_lock);
  return _text.get_plain_wtext();
}

/**
 * Returns the text currently displayed within the entry.
 */
INLINE std::wstring PGEntry::
get_wtext() const {
  LightReMutexHolder holder(_lock);
  return _text.get_wtext();
}

/**
 * Sets whether the input may be accepted--use to disable submission by the
 * user
 */
INLINE void PGEntry::
set_accept_enabled(bool enabled) {
  LightReMutexHolder holder(_lock);
  _accept_enabled = enabled;
}
