// Filename: chatInput.cxx
// Created by:  mike (09Jan97)
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

#include "chatInput.h"

#include "buttonEventDataTransition.h"
#include "buttonEvent.h"
#include "keyboardButton.h"
#include "throw_event.h"

////////////////////////////////////////////////////////////////////
// Static variables
////////////////////////////////////////////////////////////////////
TypeHandle ChatInput::_type_handle;

TypeHandle ChatInput::_button_events_type;

////////////////////////////////////////////////////////////////////
//     Function: ChatInput::Constructor
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
ChatInput::
ChatInput(TextNode* text_node, const string& name) : DataNode(name) {
  assert(text_node != NULL);
  _text_node = text_node;
  _max_chars = 0;
  _max_lines = 0;
  _max_width = 0.0;
  _flags = 0;
}

////////////////////////////////////////////////////////////////////
//     Function: ChatInput::reset
//       Access: Public
//  Description: Empties the string and prepares to accept new input;
//               does not reset the max_chars setting.
////////////////////////////////////////////////////////////////////
void ChatInput::
reset() {
  _str = "";
  _text_node->set_text(_str);
}

////////////////////////////////////////////////////////////////////
//     Function: ChatInput::transmit_data
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
void ChatInput::
transmit_data(AllTransitionsWrapper &data) {
  // Look for keyboard events.
  const ButtonEventDataTransition *b;
  if (get_transition_into(b, data, _button_events_type)) {
    ButtonEventDataTransition::const_iterator bi;
    for (bi = b->begin(); bi != b->end(); ++bi) {
      const ButtonEvent &be = (*bi);

      if (be._down) {
        if (be._button == KeyboardButton::enter()) {
          throw_event("chat_exit");

        } else if (be._button == KeyboardButton::backspace()) {
          if (!_str.empty()) {
            _str = _str.substr(0, _str.length()-1);
            _text_node->set_text(_str);
          }

        } else if (be._button.has_ascii_equivalent()) {
          char ch = be._button.get_ascii_equivalent();

          if (isprint(ch)) {
            if (!append_character(ch)) {
              throw_event("chat_overflow");
            }
          }
        }
      }
    }
  }
}

////////////////////////////////////////////////////////////////////
//     Function: init_type
//       Access:
//  Description:
////////////////////////////////////////////////////////////////////
void ChatInput::
init_type(void) {
  DataNode::init_type();
  register_type(_type_handle, "ChatInput",
                DataNode::get_class_type());

  ButtonEventDataTransition::init_type();
  register_data_transition(_button_events_type, "ButtonEvents",
                           ButtonEventDataTransition::get_class_type());
}

////////////////////////////////////////////////////////////////////
//     Function: append
//       Access: Public
//  Description: Appends the indicated string to the end of the
//               currently typed string, as if it were typed by the
//               user.  No bounds checking is performed.
////////////////////////////////////////////////////////////////////
void ChatInput::
append(const string &str) {
  _str += str;
  _text_node->set_text(_str);
}


////////////////////////////////////////////////////////////////////
//     Function: append_character
//       Access: Public
//  Description: Adds the indicated character to the end of the
//               string, as if it were typed.  Bounds checking is
//               performed; the character must fit within the limits
//               set by set_max_chars(), set_max_width(), and
//               set_max_lines().  Returns true if the character fit
//               (and was appended correctly), or false if it did not
//               fit (in which case nothing is changed).
////////////////////////////////////////////////////////////////////
bool ChatInput::
append_character(char ch) {
  if (has_max_chars() && (int)_str.size() >= get_max_chars()) {
    // This is an easy test.
    return false;
  }

  string text = _str + ch;
  if (_text_node->has_wordwrap()) {
    text =
      _text_node->wordwrap_to(text, _text_node->get_wordwrap(), false);
  }

  if (has_max_width()) {
    nassertr(!_text_node->has_wordwrap(), false);

    float width = _text_node->calc_width(text);
    if (width > get_max_width()) {
      return false;
    }
  }

  if (has_max_lines()) {
    // Count up the number of lines in the text.  This is one more
    // than the number of newline characters.
    int num_lines = 1;
    string::const_iterator pi;
    for (pi = text.begin(); pi != text.end(); ++pi) {
      if (*pi == '\n') {
        ++num_lines;
      }
    }

    if (num_lines > get_max_lines()) {
      return false;
    }
  }

  _str += ch;
  _text_node->set_text(_str);
  return true;
}
