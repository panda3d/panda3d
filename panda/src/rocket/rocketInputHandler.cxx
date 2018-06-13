/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file rocketInputHandler.cxx
 * @author rdb
 * @date 2011-12-20
 */

#include "rocketInputHandler.h"
#include "buttonEventList.h"
#include "dataGraphTraverser.h"
#include "linmath_events.h"
#include "rocketRenderInterface.h"
#include "keyboardButton.h"
#include "mouseButton.h"

#ifndef CPPPARSER
#include <Rocket/Core/Input.h>

using namespace Rocket::Core::Input;
#endif

TypeHandle RocketInputHandler::_type_handle;

/**
 *
 */
RocketInputHandler::
RocketInputHandler(const std::string &name) :
  DataNode(name),
  _mouse_xy(-1),
  _mouse_xy_changed(false),
  _modifiers(0),
  _wheel_delta(0)
{
  _pixel_xy_input = define_input("pixel_xy", EventStoreVec2::get_class_type());
  _button_events_input = define_input("button_events", ButtonEventList::get_class_type());
}

/**
 *
 */
RocketInputHandler::
~RocketInputHandler() {
}

/**
 * Returns the libRocket KeyIdentifier for the given ButtonHandle, or
 * KI_UNKNOWN (0) if it wasn't known.
 */
int RocketInputHandler::
get_rocket_key(const ButtonHandle handle) {
  static pmap<int, int> keymap;
  pmap<int, int>::const_iterator it;

  if (keymap.size() > 0) {
    it = keymap.find(handle.get_index());

    if (it == keymap.end()) {
      return 0;
    } else {
      return it->second;
    }
  }

  keymap[KeyboardButton::space().get_index()]        = KI_SPACE;
  keymap[KeyboardButton::backspace().get_index()]    = KI_BACK;
  keymap[KeyboardButton::tab().get_index()]          = KI_TAB;
  keymap[KeyboardButton::enter().get_index()]        = KI_RETURN;
  keymap[KeyboardButton::escape().get_index()]       = KI_ESCAPE;
  keymap[KeyboardButton::end().get_index()]          = KI_END;
  keymap[KeyboardButton::home().get_index()]         = KI_HOME;
  keymap[KeyboardButton::left().get_index()]         = KI_LEFT;
  keymap[KeyboardButton::up().get_index()]           = KI_UP;
  keymap[KeyboardButton::right().get_index()]        = KI_RIGHT;
  keymap[KeyboardButton::down().get_index()]         = KI_DOWN;
  keymap[KeyboardButton::insert().get_index()]       = KI_INSERT;
  keymap[KeyboardButton::del().get_index()]          = KI_DELETE;
  keymap[KeyboardButton::caps_lock().get_index()]    = KI_CAPITAL;
  keymap[KeyboardButton::f1().get_index()]           = KI_F1;
  keymap[KeyboardButton::f10().get_index()]          = KI_F10;
  keymap[KeyboardButton::f11().get_index()]          = KI_F11;
  keymap[KeyboardButton::f12().get_index()]          = KI_F12;
  keymap[KeyboardButton::f13().get_index()]          = KI_F13;
  keymap[KeyboardButton::f14().get_index()]          = KI_F14;
  keymap[KeyboardButton::f15().get_index()]          = KI_F15;
  keymap[KeyboardButton::f16().get_index()]          = KI_F16;
  keymap[KeyboardButton::f2().get_index()]           = KI_F2;
  keymap[KeyboardButton::f3().get_index()]           = KI_F3;
  keymap[KeyboardButton::f4().get_index()]           = KI_F4;
  keymap[KeyboardButton::f5().get_index()]           = KI_F5;
  keymap[KeyboardButton::f6().get_index()]           = KI_F6;
  keymap[KeyboardButton::f7().get_index()]           = KI_F7;
  keymap[KeyboardButton::f8().get_index()]           = KI_F8;
  keymap[KeyboardButton::f9().get_index()]           = KI_F9;
  keymap[KeyboardButton::help().get_index()]         = KI_HELP;
  keymap[KeyboardButton::lcontrol().get_index()]     = KI_LCONTROL;
  keymap[KeyboardButton::lshift().get_index()]       = KI_LSHIFT;
  keymap[KeyboardButton::num_lock().get_index()]     = KI_NUMLOCK;
  keymap[KeyboardButton::page_down().get_index()]    = KI_NEXT;
  keymap[KeyboardButton::page_up().get_index()]      = KI_PRIOR;
  keymap[KeyboardButton::pause().get_index()]        = KI_PAUSE;
  keymap[KeyboardButton::print_screen().get_index()] = KI_SNAPSHOT;
  keymap[KeyboardButton::rcontrol().get_index()]     = KI_RCONTROL;
  keymap[KeyboardButton::rshift().get_index()]       = KI_RSHIFT;
  keymap[KeyboardButton::scroll_lock().get_index()]  = KI_SCROLL;

  // these "OEM" keys have standard mappings in Panda3D
  keymap[KeyboardButton::ascii_key(';').get_index()]  = KI_OEM_1;
  keymap[KeyboardButton::ascii_key('=').get_index()]  = KI_OEM_PLUS;
  keymap[KeyboardButton::ascii_key(',').get_index()]  = KI_OEM_COMMA;
  keymap[KeyboardButton::ascii_key('-').get_index()]  = KI_OEM_MINUS;
  keymap[KeyboardButton::ascii_key('.').get_index()]  = KI_OEM_PERIOD;
  keymap[KeyboardButton::ascii_key('/').get_index()]  = KI_OEM_2;
  keymap[KeyboardButton::ascii_key('`').get_index()]  = KI_OEM_3;
  keymap[KeyboardButton::ascii_key('[').get_index()]  = KI_OEM_4;
  keymap[KeyboardButton::ascii_key('\\').get_index()] = KI_OEM_5;
  keymap[KeyboardButton::ascii_key(']').get_index()]  = KI_OEM_6;

  // comment says this may either be "<>" or "\|", but "\" (unshifted) is
  // handled already, and "<" is only available "shifted" on 101-keyboards, so
  // assume it's this one...
  keymap[KeyboardButton::ascii_key('<').get_index()]  = KI_OEM_102;

  for (char c = 'a'; c <= 'z'; ++c) {
    keymap[KeyboardButton::ascii_key(c).get_index()] = (c - 'a') + KI_A;
  }
  for (char c = '0'; c <= '9'; ++c) {
    keymap[KeyboardButton::ascii_key(c).get_index()] = (c - '0') + KI_0;
  }

  it = keymap.find(handle.get_index());
  if (it != keymap.end()) {
    return it->second;
  }
  return 0;
}

/**
 * The virtual implementation of transmit_data().  This function receives an
 * array of input parameters and should generate an array of output
 * parameters.  The input parameters may be accessed with the index numbers
 * returned by the define_input() calls that were made earlier (presumably in
 * the constructor); likewise, the output parameters should be set with the
 * index numbers returned by the define_output() calls.
 */
void RocketInputHandler::
do_transmit_data(DataGraphTraverser *trav, const DataNodeTransmit &input,
                 DataNodeTransmit &output) {
  MutexHolder holder(_lock);

  if (input.has_data(_pixel_xy_input)) {
    // The mouse is within the window.  Get the current mouse position.
    const EventStoreVec2 *pixel_xy;
    DCAST_INTO_V(pixel_xy, input.get_data(_pixel_xy_input).get_ptr());
    LVecBase2 p = pixel_xy->get_value();

    // Determine if mouse moved from last position
    if (p != _mouse_xy) {
      _mouse_xy_changed = true;
      _mouse_xy = p;
    }
  }

  ButtonEventList new_button_events;

  // Look for new button events.
  if (input.has_data(_button_events_input)) {
    const ButtonEventList *this_button_events;
    DCAST_INTO_V(this_button_events, input.get_data(_button_events_input).get_ptr());
    int num_events = this_button_events->get_num_events();
    for (int i = 0; i < num_events; i++) {
      const ButtonEvent &be = this_button_events->get_event(i);

      int rocket_key = KI_UNKNOWN;

      switch (be._type) {
      case ButtonEvent::T_down:
        if (be._button == KeyboardButton::control()) {
          _modifiers |= KM_CTRL;
        } else if (be._button == KeyboardButton::shift()) {
          _modifiers |= KM_SHIFT;
        } else if (be._button == KeyboardButton::alt()) {
          _modifiers |= KM_ALT;
        } else if (be._button == KeyboardButton::meta()) {
          _modifiers |= KM_META;

        } else if (be._button == KeyboardButton::enter()) {
          _text_input.push_back('\n');

        } else if (be._button == MouseButton::wheel_up()) {
          _wheel_delta -= 1;
        } else if (be._button == MouseButton::wheel_down()) {
          _wheel_delta += 1;

        } else if (be._button == MouseButton::one()) {
          _mouse_buttons[0] = true;
        } else if (be._button == MouseButton::two()) {
          _mouse_buttons[1] = true;
        } else if (be._button == MouseButton::three()) {
          _mouse_buttons[2] = true;
        } else if (be._button == MouseButton::four()) {
          _mouse_buttons[3] = true;
        } else if (be._button == MouseButton::five()) {
          _mouse_buttons[4] = true;
        }

        rocket_key = get_rocket_key(be._button);
        if (rocket_key != KI_UNKNOWN) {
          _keys[rocket_key] = true;
        }
        break;

      case ButtonEvent::T_repeat:
        if (be._button == KeyboardButton::enter()) {
          _text_input.push_back('\n');
        }

        rocket_key = get_rocket_key(be._button);
        if (rocket_key != KI_UNKNOWN) {
          _repeated_keys.push_back(rocket_key);
        }
        break;

      case ButtonEvent::T_up:
        if (be._button == KeyboardButton::control()) {
          _modifiers &= ~KM_CTRL;
        } else if (be._button == KeyboardButton::shift()) {
          _modifiers &= ~KM_SHIFT;
        } else if (be._button == KeyboardButton::alt()) {
          _modifiers &= ~KM_ALT;
        } else if (be._button == KeyboardButton::meta()) {
          _modifiers &= ~KM_META;

        } else if (be._button == MouseButton::one()) {
          _mouse_buttons[0] = false;
        } else if (be._button == MouseButton::two()) {
          _mouse_buttons[1] = false;
        } else if (be._button == MouseButton::three()) {
          _mouse_buttons[2] = false;
        } else if (be._button == MouseButton::four()) {
          _mouse_buttons[3] = false;
        } else if (be._button == MouseButton::five()) {
          _mouse_buttons[4] = false;
        }

        rocket_key = get_rocket_key(be._button);
        if (rocket_key != KI_UNKNOWN) {
          _keys[rocket_key] = false;
        }
        break;

      case ButtonEvent::T_keystroke:
        // Ignore control characters; otherwise, they actually get added to
        // strings in the UI.
        if (be._keycode > 0x1F && (be._keycode < 0x7F || be._keycode > 0x9F)) {
          _text_input.push_back(be._keycode);
        }
        break;

      case ButtonEvent::T_resume_down:
        break;

      case ButtonEvent::T_move:
        break;

      case ButtonEvent::T_candidate:
        break;

      case ButtonEvent::T_raw_down:
        break;

      case ButtonEvent::T_raw_up:
        break;
      }
    }
  }
}

/**
 * Updates the libRocket context with the changes that we have gathered in
 * do_transmit_data.  Also calls Update() on the context.
 */
void RocketInputHandler::
update_context(Rocket::Core::Context *context, int xoffs, int yoffs) {
  MutexHolder holder(_lock);

  if (_keys.size() > 0) {
    ButtonActivityMap::const_iterator it;
    for (it = _keys.begin(); it != _keys.end(); ++it) {
      if (it->second) {
        context->ProcessKeyDown((KeyIdentifier) it->first, _modifiers);
      } else {
        context->ProcessKeyUp((KeyIdentifier) it->first, _modifiers);
      }
    }
    _keys.clear();
  }

  if (_repeated_keys.size() > 0) {
    pvector<int>::const_iterator it;

    for (it = _repeated_keys.begin(); it != _repeated_keys.end(); ++it) {
      context->ProcessKeyUp((KeyIdentifier) *it, _modifiers);
      context->ProcessKeyDown((KeyIdentifier) *it, _modifiers);
    }
    _repeated_keys.clear();
  }

  if (_text_input.size() > 0) {
    pvector<short>::const_iterator it;
    for (it = _text_input.begin(); it != _text_input.end(); ++it) {
      context->ProcessTextInput(*it);
    }
    _text_input.clear();
  }

  if (_mouse_xy_changed) {
    _mouse_xy_changed = false;

    context->ProcessMouseMove(_mouse_xy.get_x() - xoffs,
                              _mouse_xy.get_y() - yoffs, _modifiers);
  }

  if (_mouse_buttons.size() > 0) {
    ButtonActivityMap::const_iterator it;
    for (it = _mouse_buttons.begin(); it != _mouse_buttons.end(); ++it) {
      if (it->second) {
        context->ProcessMouseButtonDown(it->first, _modifiers);
      } else {
        context->ProcessMouseButtonUp(it->first, _modifiers);
      }
    }
    _mouse_buttons.clear();
  }

  if (_wheel_delta != 0) {
    context->ProcessMouseWheel(_wheel_delta, _modifiers);
    _wheel_delta = 0;
  }

  context->Update();
}
