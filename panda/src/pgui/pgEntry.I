// Filename: pgEntry.I
// Created by:  drose (13Mar02)
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
//     Function: PGEntry::set_text
//       Access: Published
//  Description: Changes the text currently displayed within the
//               entry.  This uses the Unicode encoding currently
//               specified for the "focus" TextNode; therefore, the
//               TextNode must exist before calling set_text().
////////////////////////////////////////////////////////////////////
INLINE void PGEntry:: 
set_text(const string &text) {
  TextNode *text_node = get_text_def(S_focus);
  nassertv(text_node != (TextNode *)NULL);
  set_wtext(text_node->decode_text(text));
}

////////////////////////////////////////////////////////////////////
//     Function: PGEntry::get_text
//       Access: Published
//  Description: Returns the text currently displayed within the
//               entry.  This uses the Unicode encoding currently
//               specified for the "focus" TextNode; therefore, the
//               TextNode must exist before calling get_text().
////////////////////////////////////////////////////////////////////
INLINE string PGEntry:: 
get_text() const {
  TextNode *text_node = get_text_def(S_focus);
  nassertr(text_node != (TextNode *)NULL, string());
  return text_node->encode_wtext(get_wtext());
}

////////////////////////////////////////////////////////////////////
//     Function: PGEntry::set_cursor_position
//       Access: Published
//  Description: Sets the current position of the cursor.  This is the
//               position within the text at which the next letter
//               typed by the user will be inserted; normally it is
//               the same as the length of the text.
////////////////////////////////////////////////////////////////////
INLINE void PGEntry:: 
set_cursor_position(int position) {
  if (_cursor_position != position) {
    _cursor_position = position;
    _cursor_stale = true;
    _blink_start = ClockObject::get_global_clock()->get_frame_time();
  }
}

////////////////////////////////////////////////////////////////////
//     Function: PGEntry::get_cursor_position
//       Access: Published
//  Description: Returns the current position of the cursor.
////////////////////////////////////////////////////////////////////
INLINE int PGEntry:: 
get_cursor_position() const {
  return _cursor_position;
}

////////////////////////////////////////////////////////////////////
//     Function: PGEntry::set_max_chars
//       Access: Published
//  Description: Sets the maximum number of characters that may be
//               typed into the entry.  This is a limit on the number
//               of characters, as opposed to the width of the entry;
//               see also set_max_width().
//
//               If this is 0, there is no limit.
////////////////////////////////////////////////////////////////////
INLINE void PGEntry:: 
set_max_chars(int max_chars) {
  _max_chars = max_chars;
}

////////////////////////////////////////////////////////////////////
//     Function: PGEntry::get_max_chars
//       Access: Published
//  Description: Returns the current maximum number of characters that
//               may be typed into the entry, or 0 if there is no
//               limit.  See set_max_chars().
////////////////////////////////////////////////////////////////////
INLINE int PGEntry:: 
get_max_chars() const {
  return _max_chars;
}

////////////////////////////////////////////////////////////////////
//     Function: PGEntry::set_max_width
//       Access: Published
//  Description: Sets the maximum width of all characters that may be
//               typed into the entry.  This is a limit on the width
//               of the formatted text, not a fixed limit on the
//               number of characters; also set_max_chars().
//
//               If this is 0, there is no limit.
//
//               If _num_lines is more than 1, rather than being a
//               fixed width on the whole entry, this becomes instead
//               the wordwrap width (and the width limit on the entry
//               is essentially _max_width * _num_lines).
////////////////////////////////////////////////////////////////////
INLINE void PGEntry:: 
set_max_width(float max_width) {
  _max_width = max_width;
  _text_geom_stale = true;
}

////////////////////////////////////////////////////////////////////
//     Function: PGEntry::get_max_width
//       Access: Published
//  Description: Returns the current maximum width of the characters
//               that may be typed into the entry, or 0 if there is no
//               limit.  See set_max_width().
////////////////////////////////////////////////////////////////////
INLINE float PGEntry:: 
get_max_width() const {
  return _max_width;
}

////////////////////////////////////////////////////////////////////
//     Function: PGEntry::set_num_lines
//       Access: Published
//  Description: Sets the number of lines of text the PGEntry will
//               use.  This only has meaning if _max_width is not 0;
//               _max_width indicates the wordwrap width of each line.
////////////////////////////////////////////////////////////////////
INLINE void PGEntry:: 
set_num_lines(int num_lines) {
  nassertv(num_lines >= 1);
  _num_lines = num_lines;
  _text_geom_stale = true;
}

////////////////////////////////////////////////////////////////////
//     Function: PGEntry::get_num_lines
//       Access: Published
//  Description: Returns the number of lines of text the PGEntry will
//               use, if _max_width is not 0.  See set_num_lines().
////////////////////////////////////////////////////////////////////
INLINE int PGEntry:: 
get_num_lines() const {
  return _num_lines;
}

////////////////////////////////////////////////////////////////////
//     Function: PGEntry::set_blink_rate
//       Access: Published
//  Description: Sets the number of times per second the cursor will
//               blink while the entry has keyboard focus.
//
//               If this is 0, the cursor does not blink, but is held
//               steady.
////////////////////////////////////////////////////////////////////
INLINE void PGEntry:: 
set_blink_rate(float blink_rate) {
  _blink_rate = blink_rate;
}

////////////////////////////////////////////////////////////////////
//     Function: PGEntry::get_blink_rate
//       Access: Published
//  Description: Returns the number of times per second the cursor
//               will blink, or 0 if the cursor is not to blink.
////////////////////////////////////////////////////////////////////
INLINE float PGEntry:: 
get_blink_rate() const {
  return _blink_rate;
}

////////////////////////////////////////////////////////////////////
//     Function: PGEntry::get_cursor_def
//       Access: Published
//  Description: Returns the Node that will be rendered to represent
//               the cursor.  You can attach suitable cursor geometry
//               to this node.
////////////////////////////////////////////////////////////////////
INLINE const NodePath &PGEntry:: 
get_cursor_def() {
  return _cursor_def;
}

////////////////////////////////////////////////////////////////////
//     Function: PGEntry::clear_cursor_def
//       Access: Published
//  Description: Removes all the children from the cursor_def node, in
//               preparation for adding a new definition.
////////////////////////////////////////////////////////////////////
INLINE void PGEntry:: 
clear_cursor_def() {
  _cursor_def.remove_node();
  _cursor_def = _text_render_root.attach_new_node("cursor");
}

////////////////////////////////////////////////////////////////////
//     Function: PGEntry::set_cursor_keys_active
//       Access: Published
//  Description: Sets whether the arrow keys (and home/end) control
//               movement of the cursor.  If true, they are active; if
//               false, they are ignored.
////////////////////////////////////////////////////////////////////
INLINE void PGEntry:: 
set_cursor_keys_active(bool flag) {
  _cursor_keys_active = flag;
}

////////////////////////////////////////////////////////////////////
//     Function: PGEntry::get_cursor_keys_active
//       Access: Published
//  Description: Returns whether the arrow keys are currently set to
//               control movement of the cursor; see
//               set_cursor_keys_active().
////////////////////////////////////////////////////////////////////
INLINE bool PGEntry:: 
get_cursor_keys_active() const {
  return _cursor_keys_active;
}

////////////////////////////////////////////////////////////////////
//     Function: PGEntry::set_obscure_mode
//       Access: Published
//  Description: Specifies whether obscure mode should be enabled.  In
//               obscure mode, a string of asterisks is displayed
//               instead of the literal text, e.g. for entering
//               passwords.
//
//               In obscure mode, the width of the text is computed
//               based on the width of the string of asterisks, not on
//               the width of the actual text.  This has implications
//               on the maximum length of text that may be entered if
//               max_width is in effect.
////////////////////////////////////////////////////////////////////
INLINE void PGEntry:: 
set_obscure_mode(bool flag) {
  if (_obscure_mode != flag) {
    _obscure_mode = flag;
    _text_geom_stale = true;
  }
}

////////////////////////////////////////////////////////////////////
//     Function: PGEntry::get_obscure_mode
//       Access: Published
//  Description: Specifies whether obscure mode is enabled.  See
//               set_obscure_mode().
////////////////////////////////////////////////////////////////////
INLINE bool PGEntry:: 
get_obscure_mode() const {
  return _obscure_mode;
}

////////////////////////////////////////////////////////////////////
//     Function: PGEntry::get_accept_prefix
//       Access: Published, Static
//  Description: Returns the prefix that is used to define the accept
//               event for all PGEntries.  The accept event is the
//               concatenation of this string followed by get_id().
////////////////////////////////////////////////////////////////////
INLINE string PGEntry::
get_accept_prefix() {
  return "accept-";
}

////////////////////////////////////////////////////////////////////
//     Function: PGEntry::get_overflow_prefix
//       Access: Published, Static
//  Description: Returns the prefix that is used to define the overflow
//               event for all PGEntries.  The overflow event is the
//               concatenation of this string followed by get_id().
////////////////////////////////////////////////////////////////////
INLINE string PGEntry::
get_overflow_prefix() {
  return "overflow-";
}

////////////////////////////////////////////////////////////////////
//     Function: PGEntry::get_type_prefix
//       Access: Published, Static
//  Description: Returns the prefix that is used to define the type
//               event for all PGEntries.  The type event is the
//               concatenation of this string followed by get_id().
////////////////////////////////////////////////////////////////////
INLINE string PGEntry::
get_type_prefix() {
  return "type-";
}

////////////////////////////////////////////////////////////////////
//     Function: PGEntry::get_erase_prefix
//       Access: Published, Static
//  Description: Returns the prefix that is used to define the erase
//               event for all PGEntries.  The erase event is the
//               concatenation of this string followed by get_id().
////////////////////////////////////////////////////////////////////
INLINE string PGEntry::
get_erase_prefix() {
  return "erase-";
}

////////////////////////////////////////////////////////////////////
//     Function: PGEntry::get_accept_event
//       Access: Published
//  Description: Returns the event name that will be thrown when the
//               entry is accepted normally.
////////////////////////////////////////////////////////////////////
INLINE string PGEntry::
get_accept_event(const ButtonHandle &button) const {
  return "accept-" + button.get_name() + "-" + get_id();
}

////////////////////////////////////////////////////////////////////
//     Function: PGEntry::get_overflow_event
//       Access: Published
//  Description: Returns the event name that will be thrown when too
//               much text is attempted to be entered into the
//               PGEntry, exceeding either the limit set via
//               set_max_chars() or via set_max_width().
////////////////////////////////////////////////////////////////////
INLINE string PGEntry::
get_overflow_event() const {
  return "overflow-" + get_id();
}

////////////////////////////////////////////////////////////////////
//     Function: PGEntry::get_type_event
//       Access: Published
//  Description: Returns the event name that will be thrown whenever
//               the user extends the text by typing.
////////////////////////////////////////////////////////////////////
INLINE string PGEntry::
get_type_event() const {
  return "type-" + get_id();
}

////////////////////////////////////////////////////////////////////
//     Function: PGEntry::get_erase_event
//       Access: Published
//  Description: Returns the event name that will be thrown whenever
//               the user erases characters in the text.
////////////////////////////////////////////////////////////////////
INLINE string PGEntry::
get_erase_event() const {
  return "erase-" + get_id();
}

////////////////////////////////////////////////////////////////////
//     Function: PGEntry::set_wtext
//       Access: Public
//  Description: Changes the text currently displayed within the
//               entry.
////////////////////////////////////////////////////////////////////
INLINE void PGEntry:: 
set_wtext(const wstring &wtext) {
  _wtext = wtext;
  _text_geom_stale = true;
  _cursor_stale = true;
  _blink_start = ClockObject::get_global_clock()->get_frame_time();
}

////////////////////////////////////////////////////////////////////
//     Function: PGEntry::get_wtext
//       Access: Public
//  Description: Returns the text currently displayed within the
//               entry.
////////////////////////////////////////////////////////////////////
INLINE const wstring &PGEntry:: 
get_wtext() const {
  return _wtext;
}
