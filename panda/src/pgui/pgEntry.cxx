// Filename: pgEntry.cxx
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

TypeHandle PGEntry::_type_handle;

////////////////////////////////////////////////////////////////////
//     Function: PGEntry::Constructor
//       Access: Published
//  Description: 
////////////////////////////////////////////////////////////////////
PGEntry::
PGEntry(const string &name) : 
  PGItem(name)
{
  _cursor_position = 0;
  _cursor_stale = true;
  _candidate_highlight_start = 0;
  _candidate_highlight_end = 0;
  _candidate_cursor_pos = 0;
  _max_chars = 0;
  _max_width = 0.0f;
  _num_lines = 1;
  _last_text_def = (TextNode *)NULL;
  _text_geom_stale = true;
  _blink_start = 0.0f;
  _blink_rate = 1.0f;

  _text_render_root = NodePath("text_root");
  _cursor_def = _text_render_root.attach_new_node("cursor");
  _cursor_visible = true;

  // These strings are used to specify the TextProperties to apply to
  // candidate strings generated from the IME (for entering text in an
  // east Asian language).
  _candidate_active = "candidate_active";
  _candidate_inactive = "candidate_inactive";

  _cursor_keys_active = true;
  _obscure_mode = false;

  set_active(true);
  update_state();
}

////////////////////////////////////////////////////////////////////
//     Function: PGEntry::Destructor
//       Access: Public, Virtual
//  Description: 
////////////////////////////////////////////////////////////////////
PGEntry::
~PGEntry() {
}

////////////////////////////////////////////////////////////////////
//     Function: PGEntry::Copy Constructor
//       Access: Protected
//  Description: 
////////////////////////////////////////////////////////////////////
PGEntry::
PGEntry(const PGEntry &copy) :
  PGItem(copy),
  _wtext(copy._wtext),
  _obscured_wtext(copy._obscured_wtext),
  _cursor_position(copy._cursor_position),
  _max_chars(copy._max_chars),
  _max_width(copy._max_width),
  _text_defs(copy._text_defs),
  _blink_start(copy._blink_start),
  _blink_rate(copy._blink_rate),
  _cursor_keys_active(copy._cursor_keys_active),
  _obscure_mode(copy._obscure_mode)
{
  _cursor_stale = true;
  _last_text_def = (TextNode *)NULL;
  _text_geom_stale = true;
}

////////////////////////////////////////////////////////////////////
//     Function: PGEntry::make_copy
//       Access: Public, Virtual
//  Description: Returns a newly-allocated Node that is a shallow copy
//               of this one.  It will be a different Node pointer,
//               but its internal data may or may not be shared with
//               that of the original Node.
////////////////////////////////////////////////////////////////////
PandaNode *PGEntry::
make_copy() const {
  return new PGEntry(*this);
}

////////////////////////////////////////////////////////////////////
//     Function: PGEntry::has_cull_callback
//       Access: Protected, Virtual
//  Description: Should be overridden by derived classes to return
//               true if cull_callback() has been defined.  Otherwise,
//               returns false to indicate cull_callback() does not
//               need to be called for this node during the cull
//               traversal.
////////////////////////////////////////////////////////////////////
bool PGEntry::
has_cull_callback() const {
  return true;
}

////////////////////////////////////////////////////////////////////
//     Function: PGEntry::cull_callback
//       Access: Protected, Virtual
//  Description: If has_cull_callback() returns true, this function
//               will be called during the cull traversal to perform
//               any additional operations that should be performed at
//               cull time.  This may include additional manipulation
//               of render state or additional visible/invisible
//               decisions, or any other arbitrary operation.
//
//               By the time this function is called, the node has
//               already passed the bounding-volume test for the
//               viewing frustum, and the node's transform and state
//               have already been applied to the indicated
//               CullTraverserData object.
//
//               The return value is true if this node should be
//               visible, or false if it should be culled.
////////////////////////////////////////////////////////////////////
bool PGEntry::
cull_callback(CullTraverser *trav, CullTraverserData &data) {
  PGItem::cull_callback(trav, data);
  update_text();
  update_cursor();

  // Now render the text.
  CullTraverserData next_data(data, _text_render_root.node());
  trav->traverse(next_data);

  // Now continue to render everything else below this node.
  return true;
}

////////////////////////////////////////////////////////////////////
//     Function: PGEntry::press
//       Access: Public, Virtual
//  Description: This is a callback hook function, called whenever a
//               mouse or keyboard entry is depressed while the mouse
//               is within the region.
////////////////////////////////////////////////////////////////////
void PGEntry::
press(const MouseWatcherParameter &param, bool background) {
  if (get_active()) {
    if (param.has_button()) {
      ButtonHandle button = param.get_button();
      
      if (button == MouseButton::one() ||
          button == MouseButton::two() ||
          button == MouseButton::three()) {
        // Mouse button; set focus.
        set_focus(true);
        
      } else if ((!background && get_focus()) || 
                 (background && get_background_focus())) {
        // Keyboard button.
        if (!_candidate_wtext.empty()) {
          _candidate_wtext = wstring();
          _text_geom_stale = true;
        }

        _cursor_position = min(_cursor_position, (int)_wtext.length());
        _blink_start = ClockObject::get_global_clock()->get_frame_time();
        if (button == KeyboardButton::enter()) {
          // Enter.  Accept the entry.
          accept(param);
          
        } else if (button == KeyboardButton::backspace()) {
          // Backspace.  Remove the character to the left of the cursor.
          if (_cursor_position > 0) {
            if (_obscure_mode && _obscured_wtext.length() == _wtext.length()) {
              _obscured_wtext.erase(_obscured_wtext.begin() + _obscured_wtext.length() - 1);
            }
            _wtext.erase(_wtext.begin() + _cursor_position - 1);
            _cursor_position--;
            _cursor_stale = true;
            _text_geom_stale = true;
            erase(param);
          }
          
        } else if (button == KeyboardButton::del()) {
          // Delete.  Remove the character to the right of the cursor.
          if (_cursor_position < (int)_wtext.length()) {
            if (_obscure_mode && _obscured_wtext.length() == _wtext.length()) {
              _obscured_wtext.erase(_obscured_wtext.begin() + _obscured_wtext.length() - 1);
            }
            _wtext.erase(_wtext.begin() + _cursor_position);
            _text_geom_stale = true;
            erase(param);
          }
          
        } else if (button == KeyboardButton::left()) {
          if (_cursor_keys_active) {
            // Left arrow.  Move the cursor position to the left.
            _cursor_position = max(_cursor_position - 1, 0);
            _cursor_stale = true;
          }
          
        } else if (button == KeyboardButton::right()) {
          if (_cursor_keys_active) {
            // Right arrow.  Move the cursor position to the right.
            _cursor_position = min(_cursor_position + 1, (int)_wtext.length());
            _cursor_stale = true;
          }
          
        } else if (button == KeyboardButton::home()) {
          if (_cursor_keys_active) {
            // Home.  Move the cursor position to the beginning.
            _cursor_position = 0;
            _cursor_stale = true;
          }
          
        } else if (button == KeyboardButton::end()) {
          if (_cursor_keys_active) {
            // End.  Move the cursor position to the end.
            _cursor_position = _wtext.length();
            _cursor_stale = true;
          }
        }          
      }
    }
  }
  PGItem::press(param, background);
}

////////////////////////////////////////////////////////////////////
//     Function: PGEntry::keystroke
//       Access: Public, Virtual
//  Description: This is a callback hook function, called whenever
//               the user types a key.
////////////////////////////////////////////////////////////////////
void PGEntry::
keystroke(const MouseWatcherParameter &param, bool background) {
  if (get_active()) {
    if (param.has_keycode()) {
      int keycode = param.get_keycode();

      if (!isascii(keycode) || isprint(keycode)) {
        // A normal visible character.  Add a new character to the
        // text entry, if there's room.
        if (!_candidate_wtext.empty()) {
          _candidate_wtext = wstring();
          _text_geom_stale = true;
        }
        wstring new_char(1, (wchar_t)keycode);

        if (get_max_chars() > 0 && (int)_wtext.length() >= get_max_chars()) {
          // In max_chars mode, we consider it an overflow after we
          // have exceeded a fixed number of characters, irrespective
          // of the formatted width of those characters.
          overflow(param);

        } else {
          _cursor_position = min(_cursor_position, (int)_wtext.length());
          wstring new_text = 
            _wtext.substr(0, _cursor_position) + new_char +
            _wtext.substr(_cursor_position);
            
          // Get the new string to measure its length.  In normal
          // mode, we measure the text itself.  In obscure mode, we
          // measure a string of n asterisks.
          wstring measure_text;
          if (_obscure_mode) {
            measure_text = get_display_wtext() + (wchar_t)'*';
          } else {
            measure_text = new_text;
          }

          // Now check the length.
          bool too_long = false;
          if (_max_width > 0.0f) {
            TextNode *text_node = get_text_def(S_focus);
            text_node->set_wtext(measure_text);
            text_node->set_wordwrap(_max_width);
            text_node->set_preserve_trailing_whitespace(true);
            text_node->set_max_rows(_num_lines);
              
            too_long = text_node->has_overflow();
              
            if (!too_long && (text_node->get_num_rows() == _num_lines)) {
              // If we've filled up all of the available lines, we
              // must also ensure that the last line is not too long
              // (it might be, because of additional whitespace on
              // the end).
              wstring ww_text = text_node->get_wordwrapped_wtext();
              size_t last_line_start = ww_text.rfind('\n');
              if (last_line_start == string::npos) {
                last_line_start = 0;
              }
              wstring last_line = ww_text.substr(last_line_start);
              float last_line_width = text_node->calc_width(last_line);
              
              too_long = (last_line_width > _max_width);
            }

            if (!too_long && keycode == ' ') {
              // Even if we haven't filled up all of the available
              // lines, we should reject a space that's typed at the
              // end of the current line if it would make that line
              // exceed the maximum width, just so we don't allow an
              // infinite number of spaces to accumulate.

              // First, we need to figure out our current position
              // within the wordwrapped text, by skipping past the
              // newlines.
              wstring ww_text = text_node->get_wordwrapped_wtext();
              int i = 0;
              int current_pos = 0;
              while (i < _cursor_position) {
                nassertv(current_pos < (int)ww_text.length());
                if (ww_text[current_pos] != '\n') {
                  i++;
                }
                current_pos++;
              }

              // Is the user typing at the end of the line?  Scan for
              // the next character that's not a space following the
              // current position.
              int p = current_pos + 1;
              while (p < (int)ww_text.length() && ww_text[p] == ' ') {
                p++;
              }

              if (p >= (int)ww_text.length() || ww_text[p] == '\n') {
                // The user is typing at the end of the line.  But we
                // must allow at least one space at the end of the
                // line, so we only make any of the following checks
                // if there are already multiple spaces at the end of
                // the line.
                if (p - 2 >= 0 && ww_text[p - 2] == ' ') {
                  // Ok, the user is putting multiple spaces on the
                  // end of a line; we need to make sure the line does
                  // not grow too wide.  Get the beginning of the line
                  // and measure its width.
                  int q = current_pos;
                  while (q >= 0 && ww_text[q] != '\n') {
                    q--;
                  }

                  wstring current_line = ww_text.substr(q + 1, p - (q + 1));
                  float current_line_width = text_node->calc_width(current_line);

                  if (current_line_width > _max_width) {
                    // We have to reject the space, but we don't treat
                    // it as an overflow condition.  

                    // If the user is typing over existing space
                    // characters, we act as if the right-arrow key
                    // were pressed instead, and advance the cursor to
                    // the next position.  Otherwise, we just quietly
                    // eat the space character.
                    if (_cursor_position < (int)_wtext.length() && 
                        _wtext[_cursor_position] == ' ') {
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
            overflow(param);
              
          } else {
            _wtext = new_text;
            if (_obscure_mode) {
              _obscured_wtext = measure_text;
            }
              
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

////////////////////////////////////////////////////////////////////
//     Function: PGEntry::candidate
//       Access: Public, Virtual
//  Description: This is a callback hook function, called whenever
//               the user selects an item from the IME menu.
////////////////////////////////////////////////////////////////////
void PGEntry::
candidate(const MouseWatcherParameter &param, bool background) {
  if (get_active()) {
    if (param.has_candidate()) {
      // Save the candidate string so it can be displayed.
      _candidate_wtext = param.get_candidate_string();
      _candidate_highlight_start = param.get_highlight_start();
      _candidate_highlight_end = param.get_highlight_end();
      _candidate_cursor_pos = param.get_cursor_pos();
      _text_geom_stale = true;
      if (!_candidate_wtext.empty())
        type(param);
    }
  }
  PGItem::candidate(param, background);
}

////////////////////////////////////////////////////////////////////
//     Function: PGEntry::accept
//       Access: Public, Virtual
//  Description: This is a callback hook function, called whenever the
//               entry is accepted by the user pressing Enter normally.
////////////////////////////////////////////////////////////////////
void PGEntry::
accept(const MouseWatcherParameter &param) {
  PGMouseWatcherParameter *ep = new PGMouseWatcherParameter(param);
  string event = get_accept_event(param.get_button());
  play_sound(event);
  throw_event(event, EventParameter(ep));
  set_focus(false);
}

////////////////////////////////////////////////////////////////////
//     Function: PGEntry::overflow
//       Access: Public, Virtual
//  Description: This is a callback hook function, called whenever the
//               entry is overflowed because the user attempts to type
//               too many characters, exceeding either set_max_chars()
//               or set_max_width().
////////////////////////////////////////////////////////////////////
void PGEntry::
overflow(const MouseWatcherParameter &param) {
  PGMouseWatcherParameter *ep = new PGMouseWatcherParameter(param);
  string event = get_overflow_event();
  play_sound(event);
  throw_event(event, EventParameter(ep));
}

////////////////////////////////////////////////////////////////////
//     Function: PGEntry::type
//       Access: Public, Virtual
//  Description: This is a callback hook function, called whenever the
//               user extends the text by typing.
////////////////////////////////////////////////////////////////////
void PGEntry::
type(const MouseWatcherParameter &param) {
  PGMouseWatcherParameter *ep = new PGMouseWatcherParameter(param);
  string event = get_type_event();
  play_sound(event);
  throw_event(event, EventParameter(ep));
}

////////////////////////////////////////////////////////////////////
//     Function: PGEntry::erase
//       Access: Public, Virtual
//  Description: This is a callback hook function, called whenever the
//               user erase characters in the text.
////////////////////////////////////////////////////////////////////
void PGEntry::
erase(const MouseWatcherParameter &param) {
  PGMouseWatcherParameter *ep = new PGMouseWatcherParameter(param);
  string event = get_erase_event();
  play_sound(event);
  throw_event(event, EventParameter(ep));
}

////////////////////////////////////////////////////////////////////
//     Function: PGEntry::setup
//       Access: Published
//  Description: Sets up the entry for normal use.  The width is the
//               maximum width of characters that will be typed, and
//               num_lines is the integer number of lines of text of
//               the entry.  Both of these together determine the size
//               of the entry, based on the TextNode in effect.
////////////////////////////////////////////////////////////////////
void PGEntry::
setup(float width, int num_lines) {
  set_text(string());
  _cursor_position = 0;
  set_max_chars(0);
  set_max_width(width);
  set_num_lines(num_lines);

  TextNode *text_node = get_text_def(S_focus);
  float line_height = text_node->get_line_height();

  // Determine the four corners of the frame.
  LPoint3f ll(0.0f, 0.0f, -0.3f * line_height - (line_height * (num_lines - 1)));
  LPoint3f ur(width, 0.0f, line_height);
  LPoint3f lr(ur[0], 0.0f, ll[2]);
  LPoint3f ul(ll[0], 0.0f, ur[2]);

  // Transform each corner by the TextNode's transform.
  LMatrix4f mat = text_node->get_transform();
  ll = ll * mat;
  ur = ur * mat;
  lr = lr * mat;
  ul = ul * mat;

  // And get the new minmax to define the frame.  We do all this work
  // instead of just using the lower-left and upper-right corners,
  // just in case the text was rotated.
  LVecBase4f frame;
  frame[0] = min(min(ll[0], ur[0]), min(lr[0], ul[0]));
  frame[1] = max(max(ll[0], ur[0]), max(lr[0], ul[0]));
  frame[2] = min(min(ll[2], ur[2]), min(lr[2], ul[2]));
  frame[3] = max(max(ll[2], ur[2]), max(lr[2], ul[2]));

  switch (text_node->get_align()) {
  case TextNode::A_left:
    // The default case.
    break;

  case TextNode::A_center:
    frame[0] = -width / 2.0;
    frame[1] = width / 2.0;
    break;

  case TextNode::A_right:
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

  // Set up a default cursor: a vertical bar.
  clear_cursor_def();

  LineSegs ls;
  ls.set_color(0.0f, 0.0f, 0.0f, 1.0f);
  ls.move_to(0.0f, 0.0f, -0.15f * line_height);
  ls.draw_to(0.0f, 0.0f, 0.85f * line_height);
  get_cursor_def().attach_new_node(ls.create());
  
  /*
  // An underscore cursor would work too.
  text_node->set_text("_");
  get_cursor_def().attach_new_node(text_node->generate());
  */
}

////////////////////////////////////////////////////////////////////
//     Function: PGEntry::set_text_def
//       Access: Published
//  Description: Changes the TextNode that will be used to render the
//               text within the entry when the entry is in the
//               indicated state.  The default if nothing is specified
//               is the same TextNode returned by
//               PGItem::get_text_node().
////////////////////////////////////////////////////////////////////
void PGEntry::
set_text_def(int state, TextNode *node) {
  nassertv(state >= 0 && state < 1000);  // Sanity check.
  if (node == (TextNode *)NULL && state >= (int)_text_defs.size()) {
    // If we're setting it to NULL, we don't need to slot a new one.
    return;
  }
  slot_text_def(state);

  _text_defs[state] = node;
}

////////////////////////////////////////////////////////////////////
//     Function: PGEntry::get_text_def
//       Access: Published
//  Description: Returns the TextNode that will be used to render the
//               text within the entry when the entry is in the
//               indicated state.  See set_text_def().
////////////////////////////////////////////////////////////////////
TextNode *PGEntry:: 
get_text_def(int state) const {
  if (state < 0 || state >= (int)_text_defs.size()) {
    // If we don't have a definition, use the global one.
    return get_text_node();
  }
  if (_text_defs[state] == (TextNode *)NULL) {
    return get_text_node();
  }
  return _text_defs[state];
}

////////////////////////////////////////////////////////////////////
//     Function: PGEntry::set_active
//       Access: Published, Virtual
//  Description: Toggles the active/inactive state of the entry.  In
//               the case of a PGEntry, this also changes its visual
//               appearance.
////////////////////////////////////////////////////////////////////
void PGEntry:: 
set_active(bool active) {
  PGItem::set_active(active);
  update_state();
}

////////////////////////////////////////////////////////////////////
//     Function: PGEntry::set_focus
//       Access: Published, Virtual
//  Description: Toggles the focus state of the entry.  In the case of
//               a PGEntry, this also changes its visual appearance.
////////////////////////////////////////////////////////////////////
void PGEntry:: 
set_focus(bool focus) {
  PGItem::set_focus(focus);
  _blink_start = ClockObject::get_global_clock()->get_frame_time();
  update_state();
}

////////////////////////////////////////////////////////////////////
//     Function: PGEntry::get_display_wtext
//       Access: Private
//  Description: Returns the string that should be displayed within
//               the entry.  This is normally _wtext, but it may be
//               _obscured_wtext.
////////////////////////////////////////////////////////////////////
const wstring &PGEntry::
get_display_wtext() {
  if (_obscure_mode) {
    // If obscure mode is enabled, we should just display a bunch of
    // asterisks.
    if (_obscured_wtext.length() != _wtext.length()) {
      _obscured_wtext = wstring();
      wstring::const_iterator ti;
      for (ti = _wtext.begin(); ti != _wtext.end(); ++ti) {
        _obscured_wtext += (wchar_t)'*';
      }
    }

    return _obscured_wtext;

  } else {
    // In normal, non-obscure mode, we display the actual text
    return _wtext;
  }
}

////////////////////////////////////////////////////////////////////
//     Function: PGEntry::slot_text_def
//       Access: Private
//  Description: Ensures there is a slot in the array for the given
//               text definition.
////////////////////////////////////////////////////////////////////
void PGEntry::
slot_text_def(int state) {
  while (state >= (int)_text_defs.size()) {
    _text_defs.push_back((TextNode *)NULL);
  }
}

////////////////////////////////////////////////////////////////////
//     Function: PGEntry::update_text
//       Access: Private
//  Description: Causes the PGEntry to recompute its text, if
//               necessary.
////////////////////////////////////////////////////////////////////
void PGEntry:: 
update_text() {
  TextNode *node = get_text_def(get_state());
  nassertv(node != (TextNode *)NULL);

  if (_text_geom_stale || node != _last_text_def) {
    wstring display_wtext;

    if (_candidate_wtext.empty() || _obscure_mode) {
      // If we're not trying to display a candidate string, it's easy:
      // just display the current text contents.
      display_wtext = get_display_wtext();

    } else {
      // Insert the complex sequence of characters required to show
      // the candidate string in a different color.  This gets
      // inserted at the current cursor position.
      wstring source_wtext = get_display_wtext();

      display_wtext = source_wtext.substr(0, _cursor_position);
      display_wtext += wstring(1, (wchar_t)text_push_properties_key);
      display_wtext += node->decode_text(_candidate_inactive);
      display_wtext += wstring(1, (wchar_t)text_push_properties_key);
      display_wtext += _candidate_wtext.substr(0, _candidate_highlight_start);
      display_wtext += wstring(1, (wchar_t)text_push_properties_key);
      display_wtext += node->decode_text(_candidate_active);
      display_wtext += wstring(1, (wchar_t)text_push_properties_key);
      display_wtext += _candidate_wtext.substr(_candidate_highlight_start,
                                               _candidate_highlight_end - _candidate_highlight_start);
      display_wtext += wstring(1, (wchar_t)text_pop_properties_key);
      display_wtext += _candidate_wtext.substr(_candidate_highlight_end);
      display_wtext += wstring(1, (wchar_t)text_pop_properties_key);

      display_wtext += source_wtext.substr(_cursor_position);
    }

    // We need to regenerate.
    _last_text_def = node;
    _last_text_def->set_wtext(display_wtext);
    _last_text_def->set_wordwrap(_max_width);
    _last_text_def->set_preserve_trailing_whitespace(true);
    _last_text_def->set_max_rows(_num_lines);

    // Check for multiple lines.
    wstring ww_text = _last_text_def->get_wordwrapped_wtext();

    // And chop the lines up into pieces.
    _ww_lines.clear();
    size_t p = 0;
    size_t q = ww_text.find((wchar_t)'\n');
    while (q != string::npos) {
      _ww_lines.push_back(WWLine());
      WWLine &line = _ww_lines.back();
      line._str = ww_text.substr(p, q - p);
      
      // Get the left edge of the text at this line.
      line._left = 0.0f;
      if (_last_text_def->get_align() != TextNode::A_left) {
        _last_text_def->set_wtext(line._str);
        line._left = _last_text_def->get_left();
      }
      
      p = q + 1;
      q = ww_text.find('\n', p);
    }
    _ww_lines.push_back(WWLine());
    WWLine &line = _ww_lines.back();
    line._str = ww_text.substr(p);
    
    // Get the left edge of the text at this line.
    line._left = 0.0f;
    if (_last_text_def->get_align() != TextNode::A_left) {
      _last_text_def->set_wtext(line._str);
      line._left = _last_text_def->get_left();
    }

    if (!_current_text.is_empty()) {
      _current_text.remove_node();
    }
    _current_text = 
      _text_render_root.attach_new_node(_last_text_def->generate());
    _text_geom_stale = false;
    _cursor_stale = true;
  }
}

////////////////////////////////////////////////////////////////////
//     Function: PGEntry::update_cursor
//       Access: Private
//  Description: Moves the cursor to its correct position.
////////////////////////////////////////////////////////////////////
void PGEntry:: 
update_cursor() {
  TextNode *node = get_text_def(get_state());
  nassertv(node != (TextNode *)NULL);

  if (_cursor_stale || node != _last_text_def) {
    update_text();

    _cursor_position = min(_cursor_position, (int)_wtext.length());
    _candidate_cursor_pos = min(_candidate_cursor_pos, _candidate_wtext.length());

    // Determine the row and column of the cursor.
    int row = 0;
    int column = _cursor_position + _candidate_cursor_pos;
    while (row + 1 < (int)_ww_lines.size() &&
           column > (int)_ww_lines[row]._str.length()) {
      column -= _ww_lines[row]._str.length();
      row++;
    }

    nassertv(row >= 0 && row < (int)_ww_lines.size());

    // It is possible for this to become untrue legitimately, if due
    // to a candidate string we have wordwrapped down the last part of
    // the line containing the cursor.
    //nassertv(column >= 0 && column <= (int)_ww_lines[row]._str.length());

    float width = 
      _last_text_def->calc_width(_ww_lines[row]._str.substr(0, column));
    float line_height = _last_text_def->get_line_height();

    LVecBase3f trans(_ww_lines[row]._left + width, 0.0f, -line_height * row);
    _cursor_def.set_pos(trans);

    _cursor_stale = false;
  }

  // Should the cursor be visible?
  if (!get_focus()) {
    show_hide_cursor(false);
  } else {
    double elapsed_time = 
      ClockObject::get_global_clock()->get_frame_time() - _blink_start;
    int cycle = (int)(elapsed_time * _blink_rate * 2.0f);
    bool visible = ((cycle & 1) == 0);
    show_hide_cursor(visible);
  }
}

////////////////////////////////////////////////////////////////////
//     Function: PGEntry::show_hide_cursor
//       Access: Private
//  Description: Makes the cursor visible or invisible, e.g. during a
//               blink cycle.
////////////////////////////////////////////////////////////////////
void PGEntry:: 
show_hide_cursor(bool visible) {
  if (visible != _cursor_visible) {
    if (visible) {
      _cursor_def.show();
    } else {
      _cursor_def.hide();
    }
    _cursor_visible = visible;
  }
}

////////////////////////////////////////////////////////////////////
//     Function: PGEntry::update_state
//       Access: Private
//  Description: Determines what the correct state for the PGEntry
//               should be.
////////////////////////////////////////////////////////////////////
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
