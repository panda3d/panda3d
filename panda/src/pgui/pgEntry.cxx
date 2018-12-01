/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file pgEntry.cxx
 * @author drose
 * @date 2002-03-13
 */

#include "pgEntry.h"
#include "pgMouseWatcherParameter.h"

#include "cullTraverser.h"
#include "cullTraverserData.h"
#include "throw_event.h"
#include "transformState.h"
#include "mouseWatcherParameter.h"
#include "keyboardButton.h"
#include "mouseButton.h"
#include "lineSegs.h"
#include "textEncoder.h"
#include "config_text.h"

#include <math.h>

using std::max;
using std::min;
using std::string;
using std::wstring;

TypeHandle PGEntry::_type_handle;

/**
 *
 */
PGEntry::
PGEntry(const string &name) :
  PGItem(name),
  _text(get_text_node()),
  _obscure_text(get_text_node())
{
  set_cull_callback();

  _cursor_position = 0;
  _cursor_stale = true;
  _candidate_highlight_start = 0;
  _candidate_highlight_end = 0;
  _candidate_cursor_pos = 0;
  _max_chars = 0;
  _max_width = 0.0f;
  _num_lines = 1;
  _accept_enabled = true;
  _last_text_def = nullptr;
  _text_geom_stale = true;
  _text_geom_flattened = true;
  _blink_start = 0.0f;
  _blink_rate = 1.0f;

  _text_render_root = NodePath("text_root");

  CPT(TransformState) transform = TransformState::make_mat(LMatrix4::convert_mat(CS_default, CS_zup_right));
  _text_render_root.set_transform(transform);

  _cursor_scale = _text_render_root.attach_new_node("cursor_scale");
  _cursor_def = _cursor_scale.attach_new_node("cursor");
  _cursor_visible = true;

  // These strings are used to specify the TextProperties to apply to
  // candidate strings generated from the IME (for entering text in an east
  // Asian language).
  _candidate_active = "candidate_active";
  _candidate_inactive = "candidate_inactive";

  _cursor_keys_active = true;
  _obscure_mode = false;
  _overflow_mode = false;

  _current_padding = 0.0f;

  set_active(true);
  update_state();

  // Some default parameters so it doesn't crash hard if no one calls setup().
  setup_minimal(10, 1);
}

/**
 *
 */
PGEntry::
~PGEntry() {
}

/**
 *
 */
PGEntry::
PGEntry(const PGEntry &copy) :
  PGItem(copy),
  _text(copy._text),
  _obscure_text(copy._obscure_text),
  _cursor_position(copy._cursor_position),
  _cursor_visible(copy._cursor_visible),
  _candidate_highlight_start(copy._candidate_highlight_start),
  _candidate_highlight_end(copy._candidate_highlight_end),
  _candidate_cursor_pos(copy._candidate_cursor_pos),
  _max_chars(copy._max_chars),
  _max_width(copy._max_width),
  _num_lines(copy._num_lines),
  _accept_enabled(copy._accept_enabled),
  _candidate_active(copy._candidate_active),
  _candidate_inactive(copy._candidate_inactive),
  _text_defs(copy._text_defs),
  _blink_start(copy._blink_start),
  _blink_rate(copy._blink_rate),
  _cursor_keys_active(copy._cursor_keys_active),
  _obscure_mode(copy._obscure_mode),
  _overflow_mode(copy._overflow_mode)
{
  _cursor_stale = true;
  _last_text_def = nullptr;
  _text_geom_stale = true;
  _text_geom_flattened = true;

  _text_render_root = NodePath("text_root");
  _cursor_scale = _text_render_root.attach_new_node("cursor_scale");
  _cursor_scale.set_transform(copy._cursor_scale.get_transform());
  _cursor_def = copy._cursor_def.copy_to(_cursor_scale);
}

/**
 * Returns a newly-allocated Node that is a shallow copy of this one.  It will
 * be a different Node pointer, but its internal data may or may not be shared
 * with that of the original Node.
 */
PandaNode *PGEntry::
make_copy() const {
  LightReMutexHolder holder(_lock);
  return new PGEntry(*this);
}

/**
 * Transforms the contents of this node by the indicated matrix, if it means
 * anything to do so.  For most kinds of nodes, this does nothing.
 */
void PGEntry::
xform(const LMatrix4 &mat) {
  LightReMutexHolder holder(_lock);
  PGItem::xform(mat);
  _text_render_root.set_mat(_text_render_root.get_mat() * mat);
}

/**
 * This function will be called during the cull traversal to perform any
 * additional operations that should be performed at cull time.  This may
 * include additional manipulation of render state or additional
 * visible/invisible decisions, or any other arbitrary operation.
 *
 * Note that this function will *not* be called unless set_cull_callback() is
 * called in the constructor of the derived class.  It is necessary to call
 * set_cull_callback() to indicated that we require cull_callback() to be
 * called.
 *
 * By the time this function is called, the node has already passed the
 * bounding-volume test for the viewing frustum, and the node's transform and
 * state have already been applied to the indicated CullTraverserData object.
 *
 * The return value is true if this node should be visible, or false if it
 * should be culled.
 */
bool PGEntry::
cull_callback(CullTraverser *trav, CullTraverserData &data) {
  LightReMutexHolder holder(_lock);
  PGItem::cull_callback(trav, data);
  update_text();
  update_cursor();

  // Now render the text.
  CullTraverserData next_data(data, _text_render_root.node());
  trav->traverse(next_data);

  // Now continue to render everything else below this node.
  return true;
}

/**
 * This is a callback hook function, called whenever a mouse or keyboard entry
 * is depressed while the mouse is within the region.
 */
void PGEntry::
press(const MouseWatcherParameter &param, bool background) {
  LightReMutexHolder holder(_lock);
  if (get_active()) {
    if (param.has_button()) {
      // Make sure _text is initialized properly.
      update_text();

      bool overflow_mode = get_overflow_mode() && _num_lines == 1;

      ButtonHandle button = param.get_button();

      if (button == MouseButton::one() ||
          button == MouseButton::two() ||
          button == MouseButton::three() ||
          button == MouseButton::four() ||
          button == MouseButton::five()) {
        // Mouse button; set focus.
        set_focus(true);

      } else if ((!background && get_focus()) ||
                 (background && get_background_focus())) {
        // Keyboard button.
        if (!_candidate_wtext.empty()) {
          _candidate_wtext = wstring();
          _text_geom_stale = true;
        }

        _cursor_position = min(_cursor_position, _text.get_num_characters());
        _blink_start = ClockObject::get_global_clock()->get_frame_time();
        if (button == KeyboardButton::enter()) {
          // Enter.  Accept the entry.
          if (_accept_enabled) {
            accept(param);
          }
          else {
            accept_failed(param);
          }

        } else if (button == KeyboardButton::backspace()) {
          // Backspace.  Remove the character to the left of the cursor.
          if (_cursor_position > 0) {
            _text.set_wsubstr(wstring(), _cursor_position - 1, 1);
            _cursor_position--;
            _cursor_stale = true;
            _text_geom_stale = true;
            erase(param);
          }

        } else if (button == KeyboardButton::del()) {
          // Delete.  Remove the character to the right of the cursor.
          if (_cursor_position < _text.get_num_characters()) {
            _text.set_wsubstr(wstring(), _cursor_position, 1);
            _text_geom_stale = true;
            erase(param);
          }

        } else if (button == KeyboardButton::left()) {
          if (_cursor_keys_active) {
            // Left arrow.  Move the cursor position to the left.
            --_cursor_position;
            if (_cursor_position < 0) {
              _cursor_position = 0;
              overflow(param);
            } else {
              type(param);
            }
            _cursor_stale = true;
            if (overflow_mode){
                _text_geom_stale = true;
            }
          }

        } else if (button == KeyboardButton::right()) {
          if (_cursor_keys_active) {
            // Right arrow.  Move the cursor position to the right.
            ++_cursor_position;
            if (_cursor_position > _text.get_num_characters()) {
              _cursor_position = _text.get_num_characters();
              overflow(param);
            } else {
              type(param);
            }
            _cursor_stale = true;
            if (overflow_mode){
                _text_geom_stale = true;
            }
          }

        } else if (button == KeyboardButton::home()) {
          if (_cursor_keys_active) {
            // Home.  Move the cursor position to the beginning.
            _cursor_position = 0;
            _cursor_stale = true;
            if (overflow_mode){
                _text_geom_stale = true;
            }
            type(param);
          }

        } else if (button == KeyboardButton::end()) {
          if (_cursor_keys_active) {
            // End.  Move the cursor position to the end.
            _cursor_position = _text.get_num_characters();
            _cursor_stale = true;
            if (overflow_mode){
                _text_geom_stale = true;
            }
            type(param);
          }
        }
      }
    }
  }
  PGItem::press(param, background);
}

/**
 * This is a callback hook function, called whenever the user types a key.
 */
void PGEntry::
keystroke(const MouseWatcherParameter &param, bool background) {
  LightReMutexHolder holder(_lock);
  if (get_active()) {
    if (param.has_keycode()) {
      // Make sure _text is initialized properly.
      update_text();

      int keycode = param.get_keycode();

      if (!isascii(keycode) || isprint(keycode)) {
        // A normal visible character.  Add a new character to the text entry,
        // if there's room.
        if (!_candidate_wtext.empty()) {
          _candidate_wtext = wstring();
          _text_geom_stale = true;
        }
        wstring new_char(1, (wchar_t)keycode);

        if (get_max_chars() > 0 && _text.get_num_characters() >= get_max_chars()) {
          // In max_chars mode, we consider it an overflow after we have
          // exceeded a fixed number of characters, irrespective of the
          // formatted width of those characters.
          overflow(param);

        } else {
          _cursor_position = min(_cursor_position, _text.get_num_characters());
          bool too_long = !_text.set_wsubstr(new_char, _cursor_position, 0);
          bool overflow_mode = get_overflow_mode() && _num_lines == 1;
          if(overflow_mode){
            too_long = false;
          }
          if (_obscure_mode) {
            too_long = !_obscure_text.set_wtext(wstring(_text.get_num_characters(), '*'));
          } else {
            if (!too_long && (_text.get_num_rows() == _num_lines) && !overflow_mode) {
              // If we've filled up all of the available lines, we must also
              // ensure that the last line is not too long (it might be,
              // because of additional whitespace on the end).
              int r = _num_lines - 1;
              int c = _text.get_num_cols(r);
              PN_stdfloat last_line_width =
                _text.get_xpos(r, c) - _text.get_xpos(r, 0);
              too_long = (last_line_width > _max_width);
            }

            if (!too_long && keycode == ' ' && !overflow_mode) {
              // Even if we haven't filled up all of the available lines, we
              // should reject a space that's typed at the end of the current
              // line if it would make that line exceed the maximum width,
              // just so we don't allow an infinite number of spaces to
              // accumulate.
              int r, c;
              _text.calc_r_c(r, c, _cursor_position);
              if (_text.get_num_cols(r) == c + 1) {
                // The user is typing at the end of the line.  But we must
                // allow at least one space at the end of the line, so we only
                // make any of the following checks if there are already
                // multiple spaces at the end of the line.
                if (c - 1 >= 0 && _text.get_character(r, c - 1) == ' ') {
                  // Ok, the user is putting multiple spaces on the end of a
                  // line; we need to make sure the line does not grow too
                  // wide.  Measure the line's width.
                  PN_stdfloat current_line_width =
                    _text.get_xpos(r, c + 1) - _text.get_xpos(r, 0);
                  if (current_line_width > _max_width) {
                    // We have to reject the space, but we don't treat it as
                    // an overflow condition.
                    _text.set_wsubstr(wstring(), _cursor_position, 1);
                    // If the user is typing over existing space characters,
                    // we act as if the right-arrow key were pressed instead,
                    // and advance the cursor to the next position.
                    // Otherwise, we just quietly eat the space character.
                    if (_cursor_position < _text.get_num_characters() &&
                        _text.get_character(_cursor_position) == ' ') {
                      _cursor_position++;
                      _cursor_stale = true;
                    }
                    return;
                  }
                }
              }
            }
          }

          if (too_long) {
            _text.set_wsubstr(wstring(), _cursor_position, 1);
            overflow(param);

          } else {
            _cursor_position += new_char.length();
            _cursor_stale = true;
            _text_geom_stale = true;
            type(param);
          }
        }
      }
    }
  }
  PGItem::keystroke(param, background);
}

/**
 * This is a callback hook function, called whenever the user selects an item
 * from the IME menu.
 */
void PGEntry::
candidate(const MouseWatcherParameter &param, bool background) {
  LightReMutexHolder holder(_lock);
  if (get_active()) {
    if (param.has_candidate()) {
      // Save the candidate string so it can be displayed.
      _candidate_wtext = param.get_candidate_string();
      _candidate_highlight_start = param.get_highlight_start();
      _candidate_highlight_end = param.get_highlight_end();
      _candidate_cursor_pos = param.get_cursor_pos();
      _text_geom_stale = true;
      if (!_candidate_wtext.empty()) {
        type(param);
      }
    }
  }
  PGItem::candidate(param, background);
}

/**
 * This is a callback hook function, called whenever the entry is accepted by
 * the user pressing Enter normally.
 */
void PGEntry::
accept(const MouseWatcherParameter &param) {
  LightReMutexHolder holder(_lock);
  PGMouseWatcherParameter *ep = new PGMouseWatcherParameter(param);
  string event = get_accept_event(param.get_button());
  play_sound(event);
  throw_event(event, EventParameter(ep));
  set_focus(false);
}

/**
 * This is a callback hook function, called whenever the user presses Enter
 * but we can't accept the input.
 */
void PGEntry::
accept_failed(const MouseWatcherParameter &param) {
  LightReMutexHolder holder(_lock);
  PGMouseWatcherParameter *ep = new PGMouseWatcherParameter(param);
  string event = get_accept_failed_event(param.get_button());
  play_sound(event);
  throw_event(event, EventParameter(ep));
  // set_focus(false);
}

/**
 * This is a callback hook function, called whenever the entry is overflowed
 * because the user attempts to type too many characters, exceeding either
 * set_max_chars() or set_max_width().
 */
void PGEntry::
overflow(const MouseWatcherParameter &param) {
  LightReMutexHolder holder(_lock);
  PGMouseWatcherParameter *ep = new PGMouseWatcherParameter(param);
  string event = get_overflow_event();
  play_sound(event);
  throw_event(event, EventParameter(ep));
}

/**
 * This is a callback hook function, called whenever the user extends the text
 * by typing.
 */
void PGEntry::
type(const MouseWatcherParameter &param) {
  LightReMutexHolder holder(_lock);
  PGMouseWatcherParameter *ep = new PGMouseWatcherParameter(param);
  string event = get_type_event();
  play_sound(event);
  throw_event(event, EventParameter(ep));
}

/**
 * This is a callback hook function, called whenever the user erase characters
 * in the text.
 */
void PGEntry::
erase(const MouseWatcherParameter &param) {
  LightReMutexHolder holder(_lock);
  PGMouseWatcherParameter *ep = new PGMouseWatcherParameter(param);
  string event = get_erase_event();
  play_sound(event);
  throw_event(event, EventParameter(ep));
}

/**
 * This is a callback hook function, called whenever the cursor moves.
 */
void PGEntry::
cursormove() {
  LightReMutexHolder holder(_lock);
  string event = get_cursormove_event();
  throw_event(event, EventParameter(_cursor_def.get_x()), EventParameter(_cursor_def.get_y()));
}

/**
 * Sets up the entry for normal use.  The width is the maximum width of
 * characters that will be typed, and num_lines is the integer number of lines
 * of text of the entry.  Both of these together determine the size of the
 * entry, based on the TextNode in effect.
 */
void PGEntry::
setup(PN_stdfloat width, int num_lines) {
  LightReMutexHolder holder(_lock);
  setup_minimal(width, num_lines);

  TextNode *text_node = get_text_def(S_focus);
  PN_stdfloat line_height = text_node->get_line_height();

  // Determine the four corners of the frame.
  float bottom = -0.3f * line_height - (line_height * (num_lines - 1));
  // Transform each corner by the TextNode's transform.
  LMatrix4 mat = text_node->get_transform();
  LPoint3 ll = LPoint3::rfu(0.0f, 0.0f, bottom) * mat;
  LPoint3 ur = LPoint3::rfu(width, 0.0f, line_height) * mat;
  LPoint3 lr = LPoint3::rfu(width, 0.0f, bottom) * mat;
  LPoint3 ul = LPoint3::rfu(0.0f, 0.0f, line_height) * mat;

  LVector3 up = LVector3::up();
  int up_axis;
  if (up[1]) {
    up_axis = 1;
  }
  else if (up[2]) {
    up_axis = 2;
  }
  else {
    up_axis = 0;
  }
  LVector3 right = LVector3::right();
  int right_axis;
  if (right[0]) {
    right_axis = 0;
  }
  else if (right[2]) {
    right_axis = 2;
  }
  else {
    right_axis = 1;
  }

  // And get the new minmax to define the frame.  We do all this work instead
  // of just using the lower-left and upper-right corners, just in case the
  // text was rotated.
  LVecBase4 frame;
  frame[0] = min(min(ll[right_axis], ur[right_axis]), min(lr[right_axis], ul[right_axis]));
  frame[1] = max(max(ll[right_axis], ur[right_axis]), max(lr[right_axis], ul[right_axis]));
  frame[2] = min(min(ll[up_axis], ur[up_axis]), min(lr[up_axis], ul[up_axis]));
  frame[3] = max(max(ll[up_axis], ur[up_axis]), max(lr[up_axis], ul[up_axis]));

  switch (text_node->get_align()) {
  case TextNode::A_left:
  case TextNode::A_boxed_left:
    // The default case.
    break;

  case TextNode::A_center:
  case TextNode::A_boxed_center:
    frame[0] = -width / 2.0;
    frame[1] = width / 2.0;
    break;

  case TextNode::A_right:
  case TextNode::A_boxed_right:
    frame[0] = -width;
    frame[1] = 0.0f;
    break;
  }

  set_frame(frame[0] - 0.15f, frame[1] + 0.15f, frame[2], frame[3]);

  PGFrameStyle style;
  style.set_width(0.1f, 0.1f);
  style.set_type(PGFrameStyle::T_bevel_in);
  style.set_color(0.8f, 0.8f, 0.8f, 1.0f);

  set_frame_style(S_no_focus, style);

  style.set_color(0.9f, 0.9f, 0.9f, 1.0f);
  set_frame_style(S_focus, style);

  style.set_color(0.6f, 0.6f, 0.6f, 1.0f);
  set_frame_style(S_inactive, style);
}

/**
 * Sets up the entry without creating any frame or other decoration.
 */
void PGEntry::
setup_minimal(PN_stdfloat width, int num_lines) {
  LightReMutexHolder holder(_lock);
  set_text(string());
  _cursor_position = 0;
  set_max_chars(0);
  set_max_width(width);
  set_num_lines(num_lines);
  update_text();

  _accept_enabled = true;

  TextNode *text_node = get_text_def(S_focus);
  PN_stdfloat line_height = text_node->get_line_height();

  // Set up a default cursor: a vertical bar.
  clear_cursor_def();

  LineSegs ls;
  ls.set_color(text_node->get_text_color());
  ls.move_to(0.0f, 0.0f, -0.15f * line_height);
  ls.draw_to(0.0f, 0.0f, 0.70f * line_height);
  get_cursor_def().attach_new_node(ls.create());

  /*
  // An underscore cursor would work too.
  text_node->set_text("_");
  get_cursor_def().attach_new_node(text_node->generate());
  */
}

/**
 * Changes the TextNode that will be used to render the text within the entry
 * when the entry is in the indicated state.  The default if nothing is
 * specified is the same TextNode returned by PGItem::get_text_node().
 */
void PGEntry::
set_text_def(int state, TextNode *node) {
  LightReMutexHolder holder(_lock);
  nassertv(state >= 0 && state < 1000);  // Sanity check.
  if (node == nullptr && state >= (int)_text_defs.size()) {
    // If we're setting it to NULL, we don't need to slot a new one.
    return;
  }
  slot_text_def(state);

  _text_defs[state] = node;
}

/**
 * Returns the TextNode that will be used to render the text within the entry
 * when the entry is in the indicated state.  See set_text_def().
 */
TextNode *PGEntry::
get_text_def(int state) const {
  LightReMutexHolder holder(_lock);
  if (state < 0 || state >= (int)_text_defs.size()) {
    // If we don't have a definition, use the global one.
    return get_text_node();
  }
  if (_text_defs[state] == nullptr) {
    return get_text_node();
  }
  return _text_defs[state];
}

/**
 * Toggles the active/inactive state of the entry.  In the case of a PGEntry,
 * this also changes its visual appearance.
 */
void PGEntry::
set_active(bool active) {
  LightReMutexHolder holder(_lock);
  PGItem::set_active(active);
  update_state();
}

/**
 * Toggles the focus state of the entry.  In the case of a PGEntry, this also
 * changes its visual appearance.
 */
void PGEntry::
set_focus(bool focus) {
  LightReMutexHolder holder(_lock);
  PGItem::set_focus(focus);
  _blink_start = ClockObject::get_global_clock()->get_frame_time();
  update_state();
}

/**
 * Returns true if any of the characters in the string returned by get_wtext()
 * are out of the range of an ASCII character (and, therefore, get_wtext()
 * should be called in preference to get_text()).
 */
bool PGEntry::
is_wtext() const {
  LightReMutexHolder holder(_lock);
  for (int i = 0; i < _text.get_num_characters(); ++i) {
    wchar_t ch = _text.get_character(i);
    if ((ch & ~0x7f) != 0) {
      return true;
    }
  }

  return false;
}

/**
 * Ensures there is a slot in the array for the given text definition.
 */
void PGEntry::
slot_text_def(int state) {
  while (state >= (int)_text_defs.size()) {
    _text_defs.push_back(nullptr);
  }
}

/**
 * Causes the PGEntry to recompute its text, if necessary.
 */
void PGEntry::
update_text() {
  TextNode *node = get_text_def(get_state());
  nassertv(node != nullptr);

  if (_text_geom_stale || node != _last_text_def) {
    TextProperties props = *node;
    props.set_wordwrap(_max_width);
    props.set_preserve_trailing_whitespace(true);
    _text.set_properties(props);
    _text.set_max_rows(_num_lines);

    if (node != _last_text_def) {
      // Make sure the default properties are applied to all the characters in
      // the text.
      _text.set_wtext(_text.get_wtext());
      _last_text_def = node;
    }

    _text.set_multiline_mode (!(get_overflow_mode() && _num_lines == 1));

    PT(PandaNode) assembled;
    if (_obscure_mode) {
      _obscure_text.set_properties(props);
      _obscure_text.set_max_rows(_num_lines);
      _obscure_text.set_wtext(wstring(_text.get_num_characters(), '*'));
      assembled = _obscure_text.assemble_text();

    } else if (_candidate_wtext.empty()) {
      // If we're not trying to display a candidate string, it's easy: just
      // display the current text contents.
      assembled = _text.assemble_text();

    } else {
      TextPropertiesManager *tp_mgr = TextPropertiesManager::get_global_ptr();
      TextProperties inactive = tp_mgr->get_properties(_candidate_inactive);
      TextProperties active = tp_mgr->get_properties(_candidate_active);

      // Insert the complex sequence of characters required to show the
      // candidate string in a different color.  This gets inserted at the
      // current cursor position.
      wstring cseq;
      cseq += wstring(1, (wchar_t)text_push_properties_key);
      cseq += node->decode_text(_candidate_inactive);
      cseq += wstring(1, (wchar_t)text_push_properties_key);
      cseq += _candidate_wtext.substr(0, _candidate_highlight_start);
      cseq += wstring(1, (wchar_t)text_push_properties_key);
      cseq += node->decode_text(_candidate_active);
      cseq += wstring(1, (wchar_t)text_push_properties_key);
      cseq += _candidate_wtext.substr(_candidate_highlight_start,
                                       _candidate_highlight_end - _candidate_highlight_start);
      cseq += wstring(1, (wchar_t)text_pop_properties_key);
      cseq += _candidate_wtext.substr(_candidate_highlight_end);
      cseq += wstring(1, (wchar_t)text_pop_properties_key);

      // Create a special TextAssembler to insert the candidate string.
      TextAssembler ctext(_text);
      ctext.set_wsubstr(cseq, _cursor_position, 0);
      assembled = ctext.assemble_text();
    }

    if (!_current_text.is_empty()) {
      _current_text.remove_node();
    }

    _current_text =
      _text_render_root.attach_new_node(assembled);

    _current_text.set_mat(node->get_transform());

    if (get_overflow_mode() && _num_lines == 1){
      // We determine the minimum required padding:
      PN_stdfloat cursor_graphic_pos = _text.get_xpos(0, _cursor_position);
      PN_stdfloat min_padding = (cursor_graphic_pos - _max_width);

/*
 * If the current padding would produce a caret outside the text entry, we
 * relocate it.  Here we also have to make a jump towards the center when the
 * caret is going outside the visual area and there's enough text ahead for
 * increased usability.  The amount that the caret is moved for hinting
 * depends on the OS, and the specific behavior under certain circunstances in
 * different Operating Systems is very complicated (the implementation would
 * need to "remember" the original typing starting point). For the moment we
 * are gonna use an unconditional 50% jump, this behavior is found in some Mac
 * dialogs, and it's the easiest to implement by far, while providing proven
 * usability.  PROS: Reduces the amount of scrolling while both writing and
 * navigating with arrow keys, which is desirable.  CONS: The user needs to
 * remember that heshe has exceeded the boundaries, but this happens with all
 * implementations to some degree.
 */

      if (_current_padding < min_padding || _current_padding > cursor_graphic_pos){
        _current_padding = min_padding + (cursor_graphic_pos - min_padding) * 0.5;
      }

      if (_current_padding < 0){ // Caret virtual position doesn't exceed boundaries
        _current_padding = 0;
      }

      _current_text.set_x(_current_text.get_x() - _current_padding);
      _current_text.set_scissor(NodePath(this),
      LPoint3::rfu(0, 0, -0.5), LPoint3::rfu(_max_width, 0, -0.5),
      LPoint3::rfu(_max_width, 0, 1.5), LPoint3::rfu(0, 0, 1.5));
    }

    _text_geom_stale = false;
    _text_geom_flattened = false;
    _cursor_stale = true;
  }

  // We'll flatten the text geometry only if we don't have focus.  Otherwise,
  // we assume the user may be changing it frequently.
  if (!get_focus() && !_text_geom_flattened) {
    _current_text.flatten_strong();
    _text_geom_flattened = true;
  }
}

/**
 * Moves the cursor to its correct position.
 */
void PGEntry::
update_cursor() {
  TextNode *node = get_text_def(get_state());
  nassertv(node != nullptr);
  _cursor_scale.set_mat(node->get_transform());
  _cursor_scale.set_color(node->get_text_color());

  if (_cursor_stale || node != _last_text_def) {
    update_text();

    _cursor_position = min(_cursor_position, _text.get_num_characters());

    // Determine the row and column of the cursor.
    int row, column;
    PN_stdfloat xpos, ypos;
    if (_obscure_mode) {
      _obscure_text.calc_r_c(row, column, _cursor_position);
      xpos = _obscure_text.get_xpos(row, column);
      ypos = _obscure_text.get_ypos(row, column);
    } else {
      _text.calc_r_c(row, column, _cursor_position);
      if (_cursor_position > 0 && _text.get_character(_cursor_position - 1) == '\n') {
        row += 1;
        column = 0;
      }
      xpos = _text.get_xpos(row, column);
      ypos = _text.get_ypos(row, column);
    }

    _cursor_def.set_pos(xpos - _current_padding, 0.0f, ypos);
    _cursor_stale = false;
    cursormove();

  }

  // Should the cursor be visible?
  if (!get_focus() || !_candidate_wtext.empty()) {
    show_hide_cursor(false);
  } else {
    double elapsed_time =
      ClockObject::get_global_clock()->get_frame_time() - _blink_start;
    int cycle = (int)(elapsed_time * _blink_rate * 2.0f);
    bool visible = ((cycle & 1) == 0);
    show_hide_cursor(visible);
  }
}

/**
 * Makes the cursor visible or invisible, e.g.  during a blink cycle.
 */
void PGEntry::
show_hide_cursor(bool visible) {
  if (visible != _cursor_visible) {
    if (visible) {
      _cursor_scale.show();
    } else {
      _cursor_scale.hide();
    }
    _cursor_visible = visible;
  }
}

/**
 * Determines what the correct state for the PGEntry should be.
 */
void PGEntry::
update_state() {
  if (get_active()) {
    if (get_focus()) {
      set_state(S_focus);
    } else {
      set_state(S_no_focus);
    }
  } else {
    set_state(S_inactive);
  }
}
