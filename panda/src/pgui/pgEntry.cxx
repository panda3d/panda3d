// Filename: pgEntry.cxx
// Created by:  drose (10Jul01)
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

#include "pgEntry.h"
#include "pgMouseWatcherParameter.h"

#include "throw_event.h"
#include "renderRelation.h"
#include "mouseWatcherParameter.h"
#include "directRenderTraverser.h"
#include "allTransitionsWrapper.h"
#include "transformTransition.h"
#include "pruneTransition.h"
#include "keyboardButton.h"
#include "mouseButton.h"
#include "lineSegs.h"

#include "math.h"

TypeHandle PGEntry::_type_handle;

////////////////////////////////////////////////////////////////////
//     Function: PGEntry::Constructor
//       Access: Published
//  Description: 
////////////////////////////////////////////////////////////////////
PGEntry::
PGEntry(const string &name) : PGItem(name)
{
  _cursor_position = 0;
  _cursor_stale = true;
  _max_chars = 0;
  _max_width = 0.0f;
  _num_lines = 1;
  _last_text_def = (TextNode *)NULL;
  _text_geom_stale = true;
  _blink_start = 0.0f;
  _blink_rate = 1.0f;

  _text_render_root = new NamedNode("text_root");
  _current_text_arc = (NodeRelation *)NULL;
  Node *cursor = new NamedNode("cursor");
  _cursor_def = new RenderRelation(_text_render_root, cursor);
  _cursor_visible = true;

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
//       Access: Public
//  Description: 
////////////////////////////////////////////////////////////////////
PGEntry::
PGEntry(const PGEntry &copy) :
  PGItem(copy),
  _text(copy._text),
  _obscured_text(copy._obscured_text),
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
//     Function: PGEntry::Copy Assignment Operator
//       Access: Public
//  Description: 
////////////////////////////////////////////////////////////////////
void PGEntry::
operator = (const PGEntry &copy) {
  PGItem::operator = (copy);
  _text = copy._text;
  _obscured_text = copy._obscured_text;
  _cursor_position = copy._cursor_position;
  _max_chars = copy._max_chars;
  _max_width = copy._max_width;
  _text_defs = copy._text_defs;
  _blink_start = copy._blink_start;
  _blink_rate = copy._blink_rate;

  _cursor_keys_active = copy._cursor_keys_active;
  _obscure_mode = copy._obscure_mode;

  _cursor_stale = true;
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
Node *PGEntry::
make_copy() const {
  return new PGEntry(*this);
}

////////////////////////////////////////////////////////////////////
//     Function: PGEntry::draw_item
//       Access: Public, Virtual
//  Description: Called by the PGTop's traversal to draw this
//               particular item.
////////////////////////////////////////////////////////////////////
void PGEntry::
draw_item(PGTop *top, GraphicsStateGuardian *gsg, 
          const AllTransitionsWrapper &trans) {
  PGItem::draw_item(top, gsg, trans);
  update_text();
  update_cursor();

  nassertv(_text_render_root != (Node *)NULL);

  // We'll use a normal DirectRenderTraverser to do the rendering
  // of the text.
  DirectRenderTraverser drt(gsg, RenderRelation::get_class_type());
  drt.set_view_frustum_cull(false);
  drt.traverse(_text_render_root, trans);
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
        _cursor_position = min(_cursor_position, (int)_text.length());
        _blink_start = ClockObject::get_global_clock()->get_frame_time();
        if (button == KeyboardButton::enter()) {
          // Enter.  Accept the entry.
          accept(param);
          
        } else if (button == KeyboardButton::backspace()) {
          // Backspace.  Remove the character to the left of the cursor.
          if (_cursor_position > 0) {
            if (_obscure_mode && _obscured_text.length() == _text.length()) {
              _obscured_text.erase(_obscured_text.begin() + _obscured_text.length() - 1);
            }
            _text.erase(_text.begin() + _cursor_position - 1);
            _cursor_position--;
            _cursor_stale = true;
            _text_geom_stale = true;
            erase(param);
          }
          
        } else if (button == KeyboardButton::del()) {
          // Delete.  Remove the character to the right of the cursor.
          if (_cursor_position < (int)_text.length()) {
            if (_obscure_mode && _obscured_text.length() == _text.length()) {
              _obscured_text.erase(_obscured_text.begin() + _obscured_text.length() - 1);
            }
            _text.erase(_text.begin() + _cursor_position);
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
            _cursor_position = min(_cursor_position + 1, (int)_text.length());
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
            _cursor_position = _text.length();
            _cursor_stale = true;
          }
          
        } else if (button.has_ascii_equivalent()) {
          char key = button.get_ascii_equivalent();
          if (isprint(key)) {
            // A normal visible character.  Add a new character to the
            // text entry, if there's room.
            
            if (get_max_chars() > 0 && (int)_text.length() >= get_max_chars()) {
              overflow(param);
            } else {
              string new_text = 
                _text.substr(0, _cursor_position) + key +
                _text.substr(_cursor_position);

              // Get a string to measure its length.  In normal mode,
              // we measure the text itself.  In obscure mode, we
              // measure a string of n asterisks.
              string measure_text;
              if (_obscure_mode) {
                measure_text = get_display_text() + '*';
              } else {
                measure_text = new_text;
              }
              
              // Check the length.
              bool too_long = false;
              if (_max_width > 0.0f) {
                TextNode *text_node = get_text_def(S_focus);
                if (_num_lines <= 1) {
                  // If we have only one line, we can check the length
                  // by simply measuring the width of the text.
                  too_long = (text_node->calc_width(measure_text) > _max_width);

                } else {
                  // If we have multiple lines, we have to check the
                  // length by wordwrapping it and counting up the
                  // number of lines.
                  string ww_text = text_node->wordwrap_to(measure_text, _max_width, true);
                  int num_lines = 1;
                  size_t last_line_start = 0;
                  for (size_t p = 0;
                       p < ww_text.length() && !too_long;
                       ++p) {
                    if (ww_text[p] == '\n') {
                      last_line_start = p + 1;
                      num_lines++;
                      too_long = (num_lines > _num_lines);
                    }
                  }

                  if (!too_long) {
                    // We must also ensure that the last line is not too
                    // long (it might be, because of additional
                    // whitespace on the end).
                    string last_line = ww_text.substr(last_line_start);
                    float last_line_width = text_node->calc_width(last_line);
                    if (num_lines == _num_lines) {
                      // Mainly we only care about this if we're on
                      // the very last line.
                      too_long = (last_line_width > _max_width);

                    } else {
                      // If we're not on the very last line, the width
                      // is still important, just so we don't allow an
                      // infinite number of spaces to accumulate.
                      // However, we must allow at least *one* space
                      // on the end of a line.
                      if (_text.length() >= 1 && 
                          _text[_text.length() - 1] == ' ') {
                        if (last_line_width > _max_width) {
                          // In this case, however, it's not exactly
                          // an overflow; we just want to reject the
                          // space.
                          return;
                        }
                      }
                    }
                  }
                }
              }
              
              if (too_long) {
                overflow(param);
                
              } else {
                _text = new_text;
                if (_obscure_mode) {
                  _obscured_text = measure_text;
                }
                
                _cursor_position++;
                _cursor_stale = true;
                _text_geom_stale = true;
                type(param);
              }
            }
          }
        }
      }
    }
  }
  PGItem::press(param, background);
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

  // Define determine the four corners of the frame.
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
  case TM_ALIGN_LEFT:
    // The default case.
    break;

  case TM_ALIGN_CENTER:
    frame[0] = -width / 2.0;
    frame[1] = width / 2.0;
    break;

  case TM_ALIGN_RIGHT:
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
  new RenderRelation(get_cursor_def(), ls.create());

  // An underscore cursor would work too.
  //  text_node->set_text("_");
  //  new RenderRelation(get_cursor_def(), text_node->generate());
}

////////////////////////////////////////////////////////////////////
//     Function: PGEntry::set_text_def
//       Access: Published
//  Description: Changes the TextNode that will be used to render the
//               text within the entry when the entry is in the
//               indicated state.  The default if nothing is specified
//               is the same TextNode returned by
//               PGItem::get_text_node().
//
//               It is the responsibility of the user to ensure that
//               this TextNode has been frozen by a call to freeze().
//               Passing in an unfrozen TextNode will result in
//               needless work.
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
//     Function: PGEntry::get_display_text
//       Access: Private
//  Description: Returns the string that should be displayed within
//               the entry.  This is normally _text, but it may be
//               _obscured_text.
////////////////////////////////////////////////////////////////////
const string &PGEntry::
get_display_text() {
  if (_obscure_mode) {
    // If obscure mode is enabled, we should just display a bunch of
    // asterisks.
    if (_obscured_text.length() != _text.length()) {
      _obscured_text = "";
      string::const_iterator ti;
      for (ti = _text.begin(); ti != _text.end(); ++ti) {
        _obscured_text += '*';
      }
    }

    return _obscured_text;

  } else {
    // In normal, non-obscure mode, we display the actual text.
    return _text;
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
    const string &display_text = get_display_text();

    // We need to regenerate.
    _last_text_def = node;

    if (_max_width > 0.0f && _num_lines > 1) {
      // Fold the text into multiple lines.
      string ww_text = 
        _last_text_def->wordwrap_to(display_text, _max_width, true);

      // And chop the lines up into pieces.
      _ww_lines.clear();
      size_t p = 0;
      size_t q = ww_text.find('\n');
      while (q != string::npos) {
        _ww_lines.push_back(WWLine());
        WWLine &line = _ww_lines.back();
        line._str = ww_text.substr(p, q - p);

        // Get the left edge of the text at this line.
        line._left = 0.0f;
        if (_last_text_def->get_align() != TM_ALIGN_LEFT) {
          _last_text_def->set_text(line._str);
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
      if (_last_text_def->get_align() != TM_ALIGN_LEFT) {
        _last_text_def->set_text(line._str);
        line._left = _last_text_def->get_left();
      }

      _last_text_def->set_text(ww_text);

    } else {
      // Only one line.
      _ww_lines.clear();
      _ww_lines.push_back(WWLine());
      WWLine &line = _ww_lines.back();
      line._str = display_text;

      _last_text_def->set_text(display_text);
      line._left = _last_text_def->get_left();
    }

    if (_current_text_arc != (NodeRelation *)NULL) {
      remove_arc(_current_text_arc);
    }
    PT_Node text = _last_text_def->generate();
    _current_text_arc = new RenderRelation(_text_render_root, text);
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

    _cursor_position = min(_cursor_position, (int)_text.length());

    // Determine the row and column of the cursor.
    int row = 0;
    int column = _cursor_position;
    while (row + 1 < (int)_ww_lines.size() &&
           column > (int)_ww_lines[row]._str.length()) {
      column -= _ww_lines[row]._str.length();
      row++;
    }

    nassertv(row >= 0 && row < (int)_ww_lines.size());
    nassertv(column >= 0 && column <= (int)_ww_lines[row]._str.length());

    float width = 
      _last_text_def->calc_width(_ww_lines[row]._str.substr(0, column));
    float line_height = _last_text_def->get_line_height();

    LVecBase3f trans(_ww_lines[row]._left + width, 0.0f, -line_height * row);
    LMatrix4f mat = LMatrix4f::translate_mat(trans) * node->get_transform();
    _cursor_def->set_transition(new TransformTransition(mat));

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
      // Reveal the cursor.
      _cursor_def->clear_transition(PruneTransition::get_class_type());
    } else {
      // Hide the cursor.
      _cursor_def->set_transition(new PruneTransition());
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
