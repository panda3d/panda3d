// Filename: chatInput.cxx
// Created by:  mike (09Jan97)
//
////////////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////////////
// Includes
////////////////////////////////////////////////////////////////////
#include "chatInput.h"

#include <buttonEventDataTransition.h>
#include <buttonEventDataAttribute.h>
#include <buttonEvent.h>
#include <keyboardButton.h>
#include <throw_event.h>

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
ChatInput::ChatInput(TextNode* text_node,
		     const string& name) : DataNode(name) {
  assert(text_node != NULL);
  _text_node = text_node;
  _max_chars = 0;
  _has_max_chars = false;
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
void ChatInput::transmit_data(NodeAttributes &data) {
  bool changed = false;
  
  // Look for keyboard events.
  const ButtonEventDataAttribute *b;
  if (get_attribute_into(b, data, _button_events_type)) {
    ButtonEventDataAttribute::const_iterator bi;
    for (bi = b->begin(); bi != b->end(); ++bi) {
      const ButtonEvent &be = (*bi);

      if (be._down) {
	if (be._button == KeyboardButton::enter()) {
	  throw_event("chat_exit");
	  
	} else if (be._button == KeyboardButton::backspace()) {
	  _str = _str.substr(0, _str.length()-1);
	  changed = true;
	  
	} else if (be._button.has_ascii_equivalent()) {
	  char ch = be._button.get_ascii_equivalent();
	  
	  if (isprint(ch)) {
	    if (has_max_chars() && (int)_str.size() >= get_max_chars()) {
	      throw_event("chat_overflow");
	    } else {
	      _str += ch;
	      changed = true;
	    }
	  }
	}
      }
    }
  }
  
  if (changed) {
    _text_node->set_text(_str);
  }
}

////////////////////////////////////////////////////////////////////
//     Function: init_type
//       Access:
//  Description:
////////////////////////////////////////////////////////////////////
void ChatInput::init_type(void) {
  DataNode::init_type();
  register_type(_type_handle, "ChatInput",
		DataNode::get_class_type());

  ButtonEventDataTransition::init_type();
  register_data_transition(_button_events_type, "ButtonEvents",
			   ButtonEventDataTransition::get_class_type());
}

////////////////////////////////////////////////////////////////////
//     Function: append 
//       Access:
//  Description:
////////////////////////////////////////////////////////////////////
void ChatInput::append(const string &str) {
  _str += str;
  _text_node->set_text(_str);
}
